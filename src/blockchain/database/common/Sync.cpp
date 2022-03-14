// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/database/common/Sync.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "opentxs/util/Container.hpp"

extern "C" {
#include <sodium.h>
}

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/ByteLiterals.hpp"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs::blockchain::database::common
{
struct Sync::Imp final : private util::MappedFileStorage {
    auto Load(const Chain chain, const Height height, Message& output)
        const noexcept -> bool
    {
        const auto start = static_cast<std::size_t>(height + 1);
        auto haveOne{false};
        auto total = std::size_t{};
        auto lock = SharedLock{lock_};
        const auto cb = [&](const auto key, const auto value) {
            if ((nullptr == key.data()) ||
                (sizeof(std::size_t) != key.size())) {
                throw std::runtime_error("Invalid key");
            }

            const auto height = [&] {
                auto out = std::size_t{};
                std::memcpy(&out, key.data(), key.size());

                return out;
            }();

            try {
                const auto data = Data{value};
                const auto view = get_read_view(data.index_);

                if ((nullptr == view.data()) || (0 == view.size())) {
                    throw std::runtime_error("Failed to load sync packet");
                }

                auto checksum = std::uint64_t{};

                static_assert(sizeof(checksum) == crypto_shorthash_BYTES);

                if (0 !=
                    ::crypto_shorthash(
                        reinterpret_cast<unsigned char*>(&checksum),
                        reinterpret_cast<const unsigned char*>(view.data()),
                        view.size(),
                        checksum_key_.data())) {
                    throw std::runtime_error("Failed to calculate checksum");
                }

                if (data.checksum_ != checksum) {
                    auto exclusive = boost::upgrade_to_unique_lock<Mutex>{lock};
                    reorg(chain, height - 1);
                    throw std::runtime_error("checksum failure");
                }

                if (false == output.Add(view)) { return false; }

                haveOne = true;
                total += view.size();

                return total < 4_MiB;
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return false;
            }
        };

        try {
            using Dir = storage::lmdb::LMDB::Dir;
            lmdb_.ReadFrom(ChainToSyncTable(chain), start, cb, Dir::Forward);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }

        return haveOne;
    }

    auto Reorg(const Chain chain, const Height height) const noexcept -> bool
    {
        auto lock = ExclusiveLock{lock_};

        return reorg(chain, height);
    }

    auto Store(const Chain chain, const Items& items) const noexcept -> bool
    {
        if (0 == items.size()) { return true; }

        auto lock = ExclusiveLock{lock_};

        if (const auto& first = items.front();
            first.Height() <= tips_.at(chain)) {
            const auto parent = std::max<Height>(first.Height() - 1, 0);

            if (false == reorg(chain, parent)) {
                LogError()(OT_PRETTY_CLASS())("Reorg error").Flush();

                return false;
            }
        }

        auto previous = tips_.at(chain);

        OT_ASSERT(-2 < previous);

        auto txn = lmdb_.TransactionRW();
        LogTrace()(OT_PRETTY_CLASS())("previous tip height: ")(previous)
            .Flush();

        for (const auto& item : items) {
            const auto height = item.Height();

            if (++previous != height) {
                LogError()(OT_PRETTY_CLASS())("sequence error. Got ")(
                    height)(" expected ")(previous)
                    .Flush();

                return false;
            }

            const auto dbKey = static_cast<std::size_t>(height);
            auto data = Data{};
            auto raw = Space{};

            if (false == item.Serialize(writer(raw))) {
                LogError()(OT_PRETTY_CLASS())("Failed to serialize item")
                    .Flush();

                return false;
            }

            const auto size = raw.size();
            auto write = get_write_view(txn, data.index_, size);

            if (false == write.valid(size)) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to allocate space for writing")
                    .Flush();

                return false;
            }

            std::memcpy(write.data(), raw.data(), size);

            static_assert(sizeof(data.checksum_) == crypto_shorthash_BYTES);

            if (0 != ::crypto_shorthash(
                         data.WriteChecksum(),
                         write.as<unsigned char>(),
                         write.size(),
                         checksum_key_.data())) {
                LogError()(OT_PRETTY_CLASS())("Failed to calculate checksum")
                    .Flush();

                return false;
            }

            const auto result =
                lmdb_.Store(ChainToSyncTable(chain), dbKey, data, txn);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())("Failed to update index").Flush();

                return false;
            }
        }

        const auto dbKey = static_cast<std::size_t>(chain);
        const auto tip = static_cast<Height>(items.back().Height());

        if (false == lmdb_.Store(tip_table_, dbKey, tsv(tip), txn).first) {
            LogError()(OT_PRETTY_CLASS())("Failed to update tip").Flush();

            return false;
        }

        tips_.at(chain) = tip;

        if (false == txn.Finalize(true)) {
            LogError()(OT_PRETTY_CLASS())("Finalize error").Flush();

            return false;
        }

        return true;
    }

    auto Tip(const Chain chain) const noexcept -> Height
    {
        auto lock = SharedLock{lock_};

        try {

            return static_cast<Height>(tips_.at(chain));
        } catch (...) {

            return -1;
        }
    }
    Imp(const api::Session& api,
        storage::lmdb::LMDB& lmdb,
        const UnallocatedCString& path) noexcept(false)
        : MappedFileStorage(
              lmdb,
              path,
              "sync",
              Table::Config,
              static_cast<std::size_t>(Database::Key::NextSyncAddress))
        , api_(api)
        , tip_table_(Table::SyncTips)
        , lock_()
        , tips_([&] {
            auto output = Tips{};

            for (const auto chain : opentxs::blockchain::DefinedChains()) {
                output.emplace(chain, -1);
            }

            return output;
        }())
    {
        auto cb = [&](const auto key, const auto value) {
            auto chain = std::size_t{};
            auto height = Height{};

            if (key.size() != sizeof(chain)) {
                throw std::runtime_error("Invalid key");
            }

            if (value.size() != sizeof(height)) {
                throw std::runtime_error("Invalid value");
            }

            std::memcpy(&chain, key.data(), key.size());
            std::memcpy(&height, value.data(), value.size());
            tips_.at(static_cast<Chain>(chain)) = height;

            return true;
        };
        lmdb_.Read(tip_table_, cb, LMDB::Dir::Forward);

        static_assert(checksum_key_.size() == crypto_shorthash_KEYBYTES);

        for (const auto chain : opentxs::blockchain::SupportedChains()) {
            import_genesis(chain);
        }

        import_genesis(Chain::UnitTest);
    }

private:
    using Mutex = boost::upgrade_mutex;
    using SharedLock = boost::upgrade_lock<Mutex>;
    using ExclusiveLock = boost::unique_lock<Mutex>;
    using Tips = UnallocatedMap<Chain, Height>;

    static const std::array<unsigned char, 16> checksum_key_;

    const api::Session& api_;
    const int tip_table_;
    mutable Mutex lock_;
    mutable Tips tips_;

    struct Data {
        util::IndexData index_;
        std::uint64_t checksum_;

        operator ReadView() const noexcept
        {
            return {reinterpret_cast<const char*>(this), sizeof(*this)};
        }

        auto WriteChecksum() noexcept -> unsigned char*
        {
            using Return = unsigned char;

            return reinterpret_cast<Return*>(&checksum_);
        }

        Data() noexcept
            : index_()
            , checksum_()
        {
        }
        Data(const ReadView in) noexcept(false)
            : Data()
        {
            if (in.size() != sizeof(*this)) {
                throw std::out_of_range("Invalid input data");
            }

            auto it = in.data();
            std::memcpy(&index_, it, sizeof(index_));
            std::advance(it, sizeof(index_));
            std::memcpy(&checksum_, it, sizeof(checksum_));
        }
    };

    auto import_genesis(const Chain chain) noexcept -> void
    {
        if (0 <= tips_.at(chain)) { return; }

        const auto items = [&] {
            using Params = opentxs::blockchain::params::Data;
            const auto& data = Params::Chains().at(chain);
            constexpr auto filterType = opentxs::blockchain::cfilter::Type::ES;
            auto gcs = [&] {
                const auto& filter = Params::Filters().at(chain).at(filterType);
                const auto bytes =
                    api_.Factory().Data(filter.second, StringStyle::Hex);
                const auto blockHash = api_.Factory().Data(
                    data.genesis_hash_hex_, StringStyle::Hex);
                auto output = std::unique_ptr<const opentxs::blockchain::GCS>{
                    factory::GCS(
                        api_,
                        filterType,
                        opentxs::blockchain::internal::BlockHashToFilterKey(
                            blockHash->Bytes()),
                        bytes->Bytes())};

                OT_ASSERT(output);

                return output;
            }();
            auto output = Items{};
            const auto header =
                api_.Factory().Data(data.genesis_header_hex_, StringStyle::Hex);
            const auto filter = gcs->Compressed();
            output.emplace_back(
                chain,
                0,
                filterType,
                gcs->ElementCount(),
                header->Bytes(),
                reader(filter));

            return output;
        }();
        Store(chain, items);
    }
    // WARNING make sure an exclusive lock is held
    auto reorg(const Chain chain, const Height height) const noexcept -> bool
    {
        if (0 > height) {
            LogError()(OT_PRETTY_CLASS())("Invalid height").Flush();

            return false;
        }

        auto& tip = tips_.at(chain);
        auto txn = lmdb_.TransactionRW();
        const auto table = ChainToSyncTable(chain);

        for (auto key = Height{height + 1}; key <= tip; ++key) {
            if (false ==
                lmdb_.Delete(table, static_cast<std::size_t>(key), txn)) {
                LogError()(OT_PRETTY_CLASS())("Delete error").Flush();

                return false;
            }
        }

        const auto key = static_cast<std::size_t>(chain);

        if (false == lmdb_.Store(tip_table_, key, tsv(height), txn).first) {
            LogError()(OT_PRETTY_CLASS())("Failed to update tip").Flush();

            return false;
        }

        if (false == txn.Finalize(true)) {
            LogError()(OT_PRETTY_CLASS())("Finalize error").Flush();

            return false;
        }

        tip = height;

        return true;
    }
};

const std::array<unsigned char, 16> Sync::Imp::checksum_key_{};

Sync::Sync(
    const api::Session& api,
    storage::lmdb::LMDB& lmdb,
    const UnallocatedCString& path) noexcept(false)
    : imp_(std::make_unique<Imp>(api, lmdb, path))
{
}

auto Sync::Load(const Chain chain, const Height height, Message& output)
    const noexcept -> bool
{
    return imp_->Load(chain, height, output);
}

auto Sync::Reorg(const Chain chain, const Height height) const noexcept -> bool
{
    return imp_->Reorg(chain, height);
}

auto Sync::Store(const Chain chain, const Items& items) const noexcept -> bool
{
    return imp_->Store(chain, items);
}

auto Sync::Tip(const Chain chain) const noexcept -> Height
{
    return imp_->Tip(chain);
}

Sync::~Sync() = default;
}  // namespace opentxs::blockchain::database::common
