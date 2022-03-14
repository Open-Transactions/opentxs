// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <future>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include "blockchain/database/wallet/SubchainCache.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::database::wallet
{
struct SubchainData::Imp {
    auto GetSubchainID(
        const NodeID& subaccount,
        const Subchain subchain,
        MDB_txn* tx) const noexcept -> pSubchainIndex
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetIndex(
            lock,
            subaccount,
            subchain,
            default_filter_type_,
            current_version_,
            tx);
    }
    auto GetPatterns(const SubchainIndex& id) const noexcept -> Patterns
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        try {
            const auto& key = cache_.DecodeIndex(lock, id);

            return load_patterns(lock, key, cache_.GetPatternIndex(lock, id));
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }
    auto GetUntestedPatterns(
        const SubchainIndex& subchain,
        const ReadView blockID) const noexcept -> Patterns
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        try {
            const auto& key = cache_.DecodeIndex(lock, subchain);
            const auto patterns = [&] {
                const auto all = cache_.GetPatternIndex(lock, subchain);
                const auto matches = cache_.GetMatchIndex(lock, blockID);
                auto out = UnallocatedVector<pPatternID>{};
                out.reserve(std::min(all.size(), matches.size()));
                std::set_difference(
                    std::begin(all),
                    std::end(all),
                    std::begin(matches),
                    std::end(matches),
                    std::back_inserter(out));

                return out;
            }();

            return load_patterns(lock, key, patterns);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }
    auto Reorg(
        const Lock& headerOracleLock,
        MDB_txn* tx,
        const node::HeaderOracle& headers,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool
    {
        auto lock = eLock{lock_};
        const auto post = ScopeGuard{[&] { cache_.Clear(lock); }};
        upgrade_future_.get();
        const auto [height, hash] = cache_.GetLastScanned(lock, subchain);
        auto target = block::Height{};

        if (height < lastGoodHeight) {
            LogTrace()(OT_PRETTY_CLASS())(
                "no action required for this subchain since last scanned "
                "height of ")(height)(" is below the reorg parent height ")(
                lastGoodHeight)
                .Flush();

            return true;
        } else if (height > lastGoodHeight) {
            target = lastGoodHeight;
        } else {
            target = std::max<block::Height>(lastGoodHeight - 1, 0);

            return cache_.SetLastScanned(
                lock,
                subchain,
                headers.Internal().GetPosition(
                    headerOracleLock, lastGoodHeight - 1),
                tx);
        }

        const auto position =
            headers.Internal().GetPosition(headerOracleLock, target);
        LogTrace()(OT_PRETTY_CLASS())("resetting last scanned to ")(
            position.second->asHex())(" at height ")(target)
            .Flush();

        if (false == cache_.SetLastScanned(lock, subchain, position, tx)) {
            throw std::runtime_error{"database error"};
        }

        return false;
    }
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        try {
            auto output{false};
            auto newIndices = UnallocatedVector<pPatternID>{};
            auto highest = Bip32Index{};
            auto tx = lmdb_.TransactionRW();

            for (const auto& [index, patterns] : elements) {
                const auto id = pattern_id(subchain, index);
                newIndices.emplace_back(id);
                highest = std::max(highest, index);

                for (const auto& pattern : patterns) {
                    output =
                        cache_.AddPattern(lock, id, index, reader(pattern), tx);

                    if (false == output) {
                        throw std::runtime_error{"failed to store pattern"};
                    }
                }
            }

            output = cache_.SetLastIndexed(lock, subchain, highest, tx);

            if (false == output) {
                throw std::runtime_error{"failed to update highest indexed"};
            }

            for (auto& patternID : newIndices) {
                output = cache_.AddPatternIndex(lock, subchain, patternID, tx);

                if (false == output) {
                    throw std::runtime_error{
                        "failed to store subchain pattern index"};
                }
            }

            output = tx.Finalize(true);

            if (false == output) {
                throw std::runtime_error{"failed to commit transaction"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache_.Clear(lock);

            return false;
        }
    }
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetLastIndexed(lock, subchain);
    }
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetLastScanned(lock, subchain);
    }
    auto SubchainMatchBlock(
        const SubchainIndex& subchain,
        const UnallocatedVector<std::pair<ReadView, MatchingIndices>>& results)
        const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();
        auto tx = lmdb_.TransactionRW();
        auto output{true};

        try {
            for (const auto& [blockID, indices] : results) {
                for (const auto& index : indices) {
                    const auto id = pattern_id(subchain, index);
                    output = cache_.AddMatch(lock, blockID, id, tx);

                    if (false == output) {
                        throw std::runtime_error{"Failed to add match"};
                    }
                }
            }

            output = tx.Finalize(true);

            if (false == output) {
                throw std::runtime_error{"failed to commit transaction"};
            }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache_.Clear(lock);

            return false;
        }
    }
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        if (cache_.SetLastScanned(lock, subchain, position, nullptr)) {

            return true;
        } else {
            cache_.Clear(lock);

            return false;
        }
    }

    Imp(const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::cfilter::Type filter) noexcept
        : api_(api)
        , lmdb_(lmdb)
        , default_filter_type_(filter)
        , current_version_([&] {
            auto version = std::optional<VersionNumber>{};
            lmdb_.Load(
                subchain_config_,
                tsv(database::Key::Version),
                [&](const auto bytes) {
                    if (sizeof(VersionNumber) == bytes.size()) {
                        auto& out = version.emplace();
                        std::memcpy(&out, bytes.data(), bytes.size());
                    } else if (sizeof(std::size_t) == bytes.size()) {
                        auto& out = version.emplace();
                        std::memcpy(
                            &out,
                            bytes.data(),
                            std::min(bytes.size(), sizeof(out)));
                    } else {

                        OT_FAIL;
                    }
                });

            OT_ASSERT(version.has_value());

            return version.value();
        }())
        , upgrade_promise_()
        , upgrade_future_(upgrade_promise_.get_future())
        , lock_()
        , cache_(api_, lmdb_)
    {
        api_.Network().Asio().Internal().Post(
            ThreadPool::General, [this] { upgrade(); });
    }

private:
    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    const cfilter::Type default_filter_type_;
    const VersionNumber current_version_;
    std::promise<void> upgrade_promise_;
    std::shared_future<void> upgrade_future_;
    mutable std::shared_mutex lock_;
    mutable SubchainCache cache_;

    template <typename LockType, typename Container>
    auto load_patterns(
        const LockType& lock,
        const db::SubchainID& key,
        const Container& patterns) const noexcept(false) -> Patterns
    {
        auto output = Patterns{};
        const auto& subaccount = key.SubaccountID(api_);
        const auto subchain = key.Type();

        for (const auto& id : patterns) {
            for (const auto& data : cache_.GetPattern(lock, id)) {
                output.emplace_back(Parent::Pattern{
                    {data.Index(), {subchain, subaccount}},
                    space(data.Data())});
            }
        }

        return output;
    }
    auto pattern_id(const SubchainIndex& subchain, const Bip32Index index)
        const noexcept -> pPatternID
    {

        auto preimage = api_.Factory().Data();
        preimage->Assign(subchain);
        preimage->Concatenate(&index, sizeof(index));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_index(
        const NodeID& subaccount,
        const Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version) const noexcept -> pSubchainIndex
    {
        auto preimage = api_.Factory().Data();
        preimage->Assign(subaccount);
        preimage->Concatenate(&subchain, sizeof(subchain));
        preimage->Concatenate(&type, sizeof(type));
        preimage->Concatenate(&version, sizeof(version));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }

    auto upgrade() noexcept -> void
    {
        // TODO
        // 1. read every value from Table::SubchainID
        // 2. if the filter type or version does not match the current value,
        // reindex everything

        upgrade_promise_.set_value();
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

SubchainData::SubchainData(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::cfilter::Type filter) noexcept
    : imp_(std::make_unique<Imp>(api, lmdb, filter))
{
    OT_ASSERT(imp_);
}

auto SubchainData::GetSubchainID(
    const NodeID& subaccount,
    const Subchain subchain,
    MDB_txn* tx) const noexcept -> pSubchainIndex
{
    return imp_->GetSubchainID(subaccount, subchain, tx);
}

auto SubchainData::GetPatterns(const SubchainIndex& subchain) const noexcept
    -> Patterns
{
    return imp_->GetPatterns(subchain);
}

auto SubchainData::GetUntestedPatterns(
    const SubchainIndex& subchain,
    const ReadView blockID) const noexcept -> Patterns
{
    return imp_->GetUntestedPatterns(subchain, blockID);
}

auto SubchainData::Reorg(
    const Lock& headerOracleLock,
    MDB_txn* tx,
    const node::HeaderOracle& headers,
    const SubchainIndex& subchain,
    const block::Height lastGoodHeight) const noexcept(false) -> bool
{
    return imp_->Reorg(headerOracleLock, tx, headers, subchain, lastGoodHeight);
}

auto SubchainData::SubchainAddElements(
    const SubchainIndex& subchain,
    const ElementMap& elements) const noexcept -> bool
{
    return imp_->SubchainAddElements(subchain, elements);
}

auto SubchainData::SubchainLastIndexed(
    const SubchainIndex& subchain) const noexcept -> std::optional<Bip32Index>
{
    return imp_->SubchainLastIndexed(subchain);
}

auto SubchainData::SubchainLastScanned(
    const SubchainIndex& subchain) const noexcept -> block::Position
{
    return imp_->SubchainLastScanned(subchain);
}

auto SubchainData::SubchainMatchBlock(
    const SubchainIndex& index,
    const UnallocatedVector<std::pair<ReadView, MatchingIndices>>& results)
    const noexcept -> bool
{
    return imp_->SubchainMatchBlock(index, results);
}

auto SubchainData::SubchainSetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastScanned(subchain, position);
}

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet
