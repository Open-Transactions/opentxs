// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>

#include "blockchain/database/wallet/SubchainPrivate.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_node_set.hpp>
#include <algorithm>
#include <cstring>
#include <future>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/SubchainCache.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::database::wallet
{
SubchainPrivate::SubchainPrivate(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
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

                    LogAbort()().Abort();
                }
            });

        assert_true(version.has_value());

        return version.value();
    }())
    , upgrade_promise_()
    , upgrade_future_(upgrade_promise_.get_future())
    , cache_(api_, lmdb_)
{
    // TODO hold shared_ptr<api::Session> as a member variable
}

auto SubchainPrivate::Init() noexcept -> void
{
    RunJob([me = shared_from_this()] { me->upgrade(); });
}

auto SubchainPrivate::AddElements(
    const SubchainID& subchain,
    const ElementMap& elements) noexcept -> bool
{
    upgrade_future_.get();
    auto tx = lmdb_.TransactionRW();

    if (add_elements(subchain, elements, *cache_.lock(), tx)) {

        return tx.Finalize(true);
    } else {

        return false;
    }
}

auto SubchainPrivate::add_elements(
    const SubchainID& subchain,
    const ElementMap& elements,
    SubchainCache& cache,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        auto output{false};
        auto newIndices = UnallocatedVector<ElementID>{};
        auto highest = Bip32Index{};

        for (const auto& [index, patterns] : elements) {
            const auto id = pattern_id(subchain, index);
            newIndices.emplace_back(id);
            highest = std::max(highest, index);

            for (const auto& pattern : patterns) {
                output = cache.AddPattern(id, index, reader(pattern), tx);

                if (false == output) {
                    throw std::runtime_error{"failed to store pattern"};
                }
            }
        }

        output = cache.SetLastIndexed(subchain, highest, tx);

        if (false == output) {
            throw std::runtime_error{"failed to update highest indexed"};
        }

        for (auto& patternID : newIndices) {
            output = cache.AddPatternIndex(subchain, patternID, tx);

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
        LogError()()(e.what()).Flush();
        cache.Clear();

        return false;
    }
}

auto SubchainPrivate::GetID(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain) const noexcept -> SubchainID
{
    upgrade_future_.get();
    auto tx = lmdb_.TransactionRW();
    auto out = get_id(subaccount, subchain, *cache_.lock_shared(), tx);
    tx.Finalize(true);

    return out;
}

auto SubchainPrivate::GetID(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain,
    storage::lmdb::Transaction& tx) const noexcept -> SubchainID
{
    upgrade_future_.get();

    return get_id(subaccount, subchain, *cache_.lock_shared(), tx);
}

auto SubchainPrivate::get_id(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain,
    const SubchainCache& cache,
    storage::lmdb::Transaction& tx) const noexcept -> SubchainID
{
    return cache.GetIndex(
        subaccount, subchain, default_filter_type_, current_version_, tx);
}

auto SubchainPrivate::GetLastIndexed(const SubchainID& subchain) const noexcept
    -> std::optional<Bip32Index>
{
    upgrade_future_.get();

    return cache_.lock_shared()->GetLastIndexed(subchain);
}

auto SubchainPrivate::GetLastScanned(const SubchainID& subchain) const noexcept
    -> block::Position
{
    upgrade_future_.get();

    return cache_.lock_shared()->GetLastScanned(subchain);
}

auto SubchainPrivate::GetPatterns(const SubchainID& id, alloc::Default alloc)
    const noexcept -> Patterns
{
    upgrade_future_.get();

    try {

        return get_patterns(id, *cache_.lock_shared(), alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto SubchainPrivate::get_patterns(
    const SubchainID& id,
    const SubchainCache& cache,
    alloc::Default alloc) const noexcept(false) -> Patterns
{
    auto output = Patterns{alloc};
    const auto& key = cache.DecodeIndex(id);
    const auto& subaccount = key.SubaccountID(api_);
    const auto subchain = key.Type();

    for (const auto& patternid : cache.GetPatternIndex(id)) {
        for (const auto& data : cache.GetPattern(patternid)) {
            output.emplace_back(Pattern{
                {data.Index(), {subchain, subaccount}},
                space(data.Data(), alloc)});
        }
    }

    return output;
}

auto SubchainPrivate::pattern_id(
    const SubchainID& subchain,
    const Bip32Index index) const noexcept -> ElementID
{

    auto preimage = api_.Factory().Data();
    preimage.Assign(subchain);
    preimage.Concatenate(&index, sizeof(index));

    return api_.Factory().IdentifierFromPreimage(preimage.Bytes());
}

auto SubchainPrivate::Reorg(
    const node::internal::HeaderOraclePrivate& data,
    const node::HeaderOracle& headers,
    const SubchainID& subchain,
    const block::Height lastGoodHeight,
    storage::lmdb::Transaction& tx) noexcept(false) -> bool
{
    upgrade_future_.get();

    return reorg(data, headers, subchain, lastGoodHeight, *cache_.lock(), tx);
}

auto SubchainPrivate::reorg(
    const node::internal::HeaderOraclePrivate& data,
    const node::HeaderOracle& headers,
    const SubchainID& subchain,
    const block::Height lastGoodHeight,
    SubchainCache& cache,
    storage::lmdb::Transaction& tx) noexcept(false) -> bool
{
    const auto post = ScopeGuard{[&] { cache.Clear(); }};
    const auto [height, hash] = cache.GetLastScanned(subchain);
    auto target = block::Height{};

    if (height < lastGoodHeight) {
        LogTrace()()("no action required for this subchain since last scanned "
                     "height of ")(
            height)(" is below the reorg parent height ")(lastGoodHeight)
            .Flush();

        return true;
    } else if (height > lastGoodHeight) {
        target = lastGoodHeight;
    } else {
        target = std::max<block::Height>(lastGoodHeight - 1, 0);
    }

    const auto position = headers.Internal().GetPosition(data, target);
    LogTrace()()("resetting last scanned to ")(position).Flush();

    if (false == cache.SetLastScanned(subchain, position, tx)) {
        throw std::runtime_error{"database error"};
    }

    return false;
}

auto SubchainPrivate::SetLastScanned(
    const SubchainID& subchain,
    const block::Position& position) noexcept -> bool
{
    upgrade_future_.get();
    auto tx = lmdb_.TransactionRW();

    if (set_last_scanned(subchain, position, *cache_.lock(), tx)) {

        return tx.Finalize(true);
    }

    return false;
}

auto SubchainPrivate::set_last_scanned(
    const SubchainID& subchain,
    const block::Position& position,
    SubchainCache& cache,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    if (cache.SetLastScanned(subchain, position, tx)) {

        return true;
    } else {
        cache.Clear();

        return false;
    }
}

auto SubchainPrivate::subchain_index(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain,
    const cfilter::Type type,
    const VersionNumber version) const noexcept -> SubchainID
{
    auto preimage = api_.Factory().Data();
    preimage.Assign(subaccount);
    preimage.Concatenate(&subchain, sizeof(subchain));
    preimage.Concatenate(&type, sizeof(type));
    preimage.Concatenate(&version, sizeof(version));

    return api_.Factory().AccountIDFromPreimage(
        preimage.Bytes(), identifier::AccountSubtype::blockchain_subchain);
}

auto SubchainPrivate::upgrade() noexcept -> void
{
    // TODO
    // 1. read every value from Table::SubchainID
    // 2. if the filter type or version does not match the current value,
    // reindex everything

    upgrade_promise_.set_value();
}

SubchainPrivate::~SubchainPrivate() = default;
}  // namespace opentxs::blockchain::database::wallet
