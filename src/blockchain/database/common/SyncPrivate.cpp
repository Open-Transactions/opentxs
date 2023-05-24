// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/SyncPrivate.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

extern "C" {
#include <sodium.h>
}

#include "blockchain/database/common/Database.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ByteLiterals.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::database::common
{
using SyncChecksum = std::uint64_t;
constexpr auto sync_bytes_ = storage::file::index_bytes_ + sizeof(SyncChecksum);
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
struct SyncPrivate::Data {
    storage::file::Index index_;
    SyncChecksum checksum_;

    auto Serialize() const noexcept -> FixedByteArray<sync_bytes_>
    {
        auto serialized = FixedByteArray<sync_bytes_>{};
        index_.Serialize(serialized.WriteInto());
        auto* out = static_cast<std::byte*>(serialized.data());
        std::advance(out, storage::file::index_bytes_);
        const auto buf = boost::endian::little_uint64_buf_t{checksum_};
        std::memcpy(out, std::addressof(buf), sizeof(buf));

        return serialized;
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
        static constexpr auto indexSize = storage::file::index_bytes_;
        auto buf = boost::endian::little_uint64_buf_t{};

        if (in.size() != (sizeof(buf) + indexSize)) {
            throw std::out_of_range("Invalid input size");
        }

        index_.Deserialize(in);
        auto const* const it = std::next(in.data(), indexSize);
        std::memcpy(static_cast<void*>(std::addressof(buf)), it, sizeof(buf));
        checksum_ = buf.value();
    }
};
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
SyncPrivate::SyncPrivate(
    const api::Session& api,
    storage::lmdb::Database& lmdb,
    const std::filesystem::path& path) noexcept(false)
    : Mapped(
          path,
          "sync",
          lmdb,
          Table::Config,
          static_cast<std::size_t>(common::Database::Key::NextSyncAddress),
          {})  // TODO allocator
    , api_(api)
    , tip_table_(Table::SyncTips)
    , tips_([&] {
        auto output = Tips{get_allocator()};

        for (const auto chain : opentxs::blockchain::DefinedChains()) {
            output.emplace(chain, -1);
        }

        auto cb = [&](const auto key, const auto value) {
            auto chain = 0_uz;
            auto height = block::Height{};

            if (key.size() != sizeof(chain)) {
                throw std::runtime_error("Invalid key");
            }

            if (value.size() != sizeof(height)) {
                throw std::runtime_error("Invalid value");
            }

            std::memcpy(&chain, key.data(), key.size());
            std::memcpy(&height, value.data(), value.size());
            output.at(static_cast<blockchain::Type>(chain)) = height;

            return true;
        };
        lmdb_.Read(tip_table_, cb, storage::lmdb::Dir::Forward);

        return output;
    }())
    , checksum_failure_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Publish);
        const auto rc = out.Bind(
            api_.Endpoints().Internal().BlockchainSyncChecksumFailure().data());

        OT_ASSERT(rc);

        return out;
    }())
{
    for (const auto chain : opentxs::blockchain::SupportedChains()) {
        import_genesis(chain);
    }

    import_genesis(blockchain::Type::UnitTest);
}

auto SyncPrivate::checksum_key() noexcept -> const unsigned char*
{
    static const auto buffer =
        std::array<unsigned char, crypto_shorthash_KEYBYTES>{};

    return buffer.data();
}

auto SyncPrivate::import_genesis(const blockchain::Type chain) noexcept -> void
{
    if (0 <= tips_.lock()->at(chain)) { return; }

    const auto items = [&] {
        namespace params = opentxs::blockchain::params;
        constexpr auto filterType = opentxs::blockchain::cfilter::Type::ES;
        const auto& data = params::get(chain);
        const auto& gcs = data.GenesisCfilter(api_, filterType);
        auto output = network::otdht::SyncData{};
        const auto header = [&] {
            auto out = ByteArray{};
            data.GenesisBlock().Header().Serialize(out.WriteInto());

            return out;
        }();
        const auto filter = [&] {
            auto out = Space{};
            gcs.Compressed(writer(out));

            return out;
        }();
        output.emplace_back(
            chain,
            0,
            filterType,
            gcs.ElementCount(),
            header.Bytes(),
            reader(filter));

        return output;
    }();
    Store(items, chain);
}

auto SyncPrivate::Load(
    const blockchain::Type chain,
    const block::Height height,
    network::otdht::Data& output) const noexcept -> bool
{
    static constexpr auto maxBlocks = 25000_uz;
    static const auto maxBytes = convert_to_size(1_mib);
    auto haveOne{false};

    try {
        const auto blockData = [&] {
            // TODO allocator
            auto out =
                std::pair<Vector<storage::file::Index>, Vector<SyncChecksum>>{};
            const auto cb = [&](const auto key, const auto value) {
                if ((nullptr == key.data()) ||
                    (sizeof(std::size_t) != key.size())) {
                    throw std::runtime_error("Invalid key");
                }

                try {
                    auto data = Data{value};
                    auto& [items, checksums] = out;
                    items.emplace_back(std::move(data.index_));
                    checksums.emplace_back(std::move(data.checksum_));

                    return items.size() < maxBlocks;
                } catch (const std::exception& e) {
                    LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                    return false;
                }
            };
            using Dir = storage::lmdb::Dir;
            const auto start = convert_to_size(height + 1);
            lmdb_.ReadFrom(ChainToSyncTable(chain), start, cb, Dir::Forward);

            return out;
        }();
        const auto& [items, checksums] = blockData;
        const auto files = Read(items, {});

        OT_ASSERT(items.size() == checksums.size());
        OT_ASSERT(items.size() == files.size());

        auto n = 0_uz;
        auto total = 0_uz;

        while ((total < maxBytes) && (n < files.size())) {
            const auto post = ScopeGuard{[&] { ++n; }};
            const auto view = files[n];
            const auto& expected = checksums[n];

            if (false == valid(view)) {
                throw std::runtime_error("Failed to load sync packet");
            }

            auto checksum = std::uint64_t{};

            static_assert(sizeof(checksum) == crypto_shorthash_BYTES);

            if (0 !=
                ::crypto_shorthash(
                    reinterpret_cast<unsigned char*>(std::addressof(checksum)),
                    reinterpret_cast<const unsigned char*>(view.data()),
                    view.size(),
                    checksum_key())) {
                throw std::runtime_error("Failed to calculate checksum");
            }

            if (expected != checksum) {
                checksum_failure_.lock()->SendDeferred(
                    [&] {
                        auto out =
                            MakeWork(OT_ZMQ_BLOCKCHAIN_SYNC_CHECKSUM_FAILURE);
                        out.AddFrame(chain);
                        out.AddFrame(height);
                        out.AddFrame(output.Version());

                        return out;
                    }(),
                    __FILE__,
                    __LINE__);

                throw std::runtime_error("checksum failure");
            }

            if (false == output.Add(view)) {
                throw std::runtime_error("failed to add block to output");
            }

            total += view.size();
            haveOne = true;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return haveOne;
}

auto SyncPrivate::Reorg(
    const blockchain::Type chain,
    const block::Height height) const noexcept -> bool
{
    return reorg(chain, height);
}

auto SyncPrivate::reorg(
    const blockchain::Type chain,
    const block::Height height) const noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();

    if (reorg(tx, chain, height)) {
        if (tx.Finalize(true)) {

            return true;
        } else {
            LogError()(OT_PRETTY_CLASS())("Finalize error").Flush();

            return false;
        }
    } else {

        return false;
    }
}

auto SyncPrivate::reorg(
    storage::lmdb::Transaction& tx,
    const blockchain::Type chain,
    const block::Height height) const noexcept -> bool
{
    if (0 > height) {
        LogError()(OT_PRETTY_CLASS())("Invalid height").Flush();

        return false;
    }

    auto handle = tips_.lock();
    auto& tip = handle->at(chain);
    const auto table = ChainToSyncTable(chain);

    for (auto key = block::Height{height + 1}; key <= tip; ++key) {
        if (false == lmdb_.Delete(table, static_cast<std::size_t>(key), tx)) {
            LogError()(OT_PRETTY_CLASS())("Delete error").Flush();

            return false;
        }
    }

    const auto key = static_cast<std::size_t>(chain);

    if (false == lmdb_.Store(tip_table_, key, tsv(height), tx).first) {
        LogError()(OT_PRETTY_CLASS())("Failed to update tip").Flush();

        return false;
    }

    tip = height;

    return true;
}

auto SyncPrivate::Store(
    const network::otdht::SyncData& items,
    blockchain::Type chain) noexcept -> bool
{
    const auto count = items.size();

    if (0_uz == count) { return true; }

    try {
        const auto [keys, bytes, sizes] = [&] {
            auto out = std::tuple<
                Vector<block::Height>,
                Vector<Space>,
                Vector<std::size_t>>{};
            auto& [k, b, s] = out;
            k.reserve(count);
            b.reserve(count);
            s.reserve(count);
            k.clear();
            b.clear();
            s.clear();
            auto previous = std::optional<block::Height>{std::nullopt};

            for (const auto& item : items) {
                const auto height = item.Height();

                if (previous.has_value()) {
                    auto& value = previous.value();

                    if (++value != height) {
                        const auto error = CString{"sequence error. Got "}
                                               .append(std::to_string(height))
                                               .append(" expected ")
                                               .append(std::to_string(value));

                        throw std::runtime_error{error.c_str()};
                    }
                } else {
                    previous.emplace(height);
                }

                const auto& dbKey = k.emplace_back(convert_to_size(height));
                auto& raw = b.emplace_back();

                if (false == item.Serialize(writer(raw))) {
                    throw std::runtime_error{"Failed to serialize item"};
                }

                const auto& size = s.emplace_back(raw.size());
                LogTrace()(OT_PRETTY_CLASS())("serialized ")(
                    size)(" bytes for ")(print(chain))(" sync data at height ")(
                    dbKey)
                    .Flush();
            }

            return out;
        }();

        OT_ASSERT(count == bytes.size());
        OT_ASSERT(count == sizes.size());

        auto tx = lmdb_.TransactionRW();

        if (const auto& first = items.front();
            first.Height() <= tips_.lock()->at(chain)) {
            const auto parent = std::max<block::Height>(first.Height() - 1, 0);

            if (false == reorg(tx, chain, parent)) {
                throw std::runtime_error{"Reorg error"};
            }
        }

        // TODO monotonic allocator
        auto in = Vector<ReadView>{};
        auto out = Vector<storage::file::Location>{};
        in.reserve(count);
        out.reserve(count);
        auto write = Write(tx, sizes);

        for (auto n = 0_uz; n < count; ++n) {
            const auto& dbKey = keys[n];
            const auto& raw = bytes[n];
            const auto& size = sizes[n];
            auto& [index, location] = write[n];
            auto& [_, view] = location;

            if (view.size() != size) {
                throw std::runtime_error{
                    "failed to get write position for sync data"};
            }

            in.emplace_back(reader(raw));
            out.emplace_back(std::move(location));
            auto data = Data{};
            data.index_ = index;

            static_assert(sizeof(data.checksum_) == crypto_shorthash_BYTES);

            if (0 != ::crypto_shorthash(
                         data.WriteChecksum(),
                         reinterpret_cast<const unsigned char*>(raw.data()),
                         raw.size(),
                         checksum_key())) {
                throw std::runtime_error{"Failed to calculate checksum"};
            }

            const auto sData = data.Serialize();
            const auto result =
                lmdb_.Store(ChainToSyncTable(chain), dbKey, sData.Bytes(), tx);

            if (result.first) {
                LogTrace()(OT_PRETTY_CLASS())("saved ")(
                    size)(" bytes at position ")(index.MemoryPosition())(
                    " for ")(print(chain))(" sync data at height ")(dbKey)
                    .Flush();
            } else {
                throw std::runtime_error{
                    "Failed to update index for block header"};
            }
        }

        // TODO monotonic allocator
        const auto written = storage::file::Mapped::Write(in, out, {});

        if (false == written) {
            throw std::runtime_error{"failed to write sync data"};
        }

        const auto dbKey = static_cast<std::size_t>(chain);
        const auto tip = static_cast<block::Height>(items.back().Height());

        if (false == lmdb_.Store(tip_table_, dbKey, tsv(tip), tx).first) {

            throw std::runtime_error{"Failed to update tip"};
        }

        tips_.lock()->at(chain) = tip;

        if (false == tx.Finalize(true)) {

            throw std::runtime_error{"finalize error"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto SyncPrivate::Tip(const blockchain::Type chain) const noexcept
    -> block::Height
{
    try {

        return tips_.lock()->at(chain);
    } catch (...) {

        return -1;
    }
}
}  // namespace opentxs::blockchain::database::common
