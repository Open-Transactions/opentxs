// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Database.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
#include <sodium.h>
}

#include <BlockchainBlockHeader.pb.h>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/database/common/BlockFilter.hpp"
#include "blockchain/database/common/BlockHeaders.hpp"
#include "blockchain/database/common/Blocks.hpp"
#include "blockchain/database/common/Bulk.hpp"
#include "blockchain/database/common/Config.hpp"
#include "blockchain/database/common/Peers.hpp"
#include "blockchain/database/common/Sync.hpp"
#include "blockchain/database/common/Wallet.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

constexpr auto false_byte_ = std::byte{0x0};
constexpr auto true_byte_ = std::byte{0x1};

namespace fs = std::filesystem;

namespace opentxs::blockchain::database::common
{
struct Database::Imp {
    using SiphashKey = Space;

    static const storage::lmdb::TableNames table_names_;

    const api::Session& api_;
    const api::Legacy& legacy_;
    const fs::path blockchain_path_;
    const fs::path common_path_;
    const fs::path blocks_path_;
    storage::lmdb::Database lmdb_;
    Bulk bulk_;
    const SiphashKey siphash_key_;
    BlockHeader headers_;
    Peers peers_;
    BlockFilter filters_;
    Blocks blocks_;
    Sync sync_;
    Wallet wallet_;
    Configuration config_;

    static auto block_storage_enabled() noexcept -> bool
    {
        return 1 == storage_enabled_;
    }
    static auto init_folder(
        const api::Legacy& legacy,
        const fs::path& parent,
        const fs::path& child) noexcept(false) -> fs::path
    {
        auto output = fs::path{};

        if (false == legacy.AppendFolder(output, parent, child)) {
            throw std::runtime_error("Failed to calculate path");
        }

        if (false == legacy.BuildFolderPath(output)) {
            throw std::runtime_error("Failed to construct path");
        }

        return output;
    }
    static auto init_storage_path(
        const api::Legacy& legacy,
        const fs::path& dataFolder) noexcept(false) -> fs::path
    {
        auto output = fs::path{};

        if (false == legacy.AppendFolder(output, dataFolder, "blockchain")) {
            throw std::runtime_error("Failed to calculate path");
        }

        constexpr auto filename{"version.3"};
        const auto base = fs::path{output};
        const auto version = base / fs::path{filename};
        const auto haveBase = [&] {
            try {

                return fs::exists(base);
            } catch (...) {

                return false;
            }
        }();
        const auto haveVersion = [&] {
            try {

                return fs::exists(version);
            } catch (...) {

                return false;
            }
        }();

        if (haveBase) {
            if (haveVersion) {
                LogVerbose()(
                    "Existing blockchain data directory already updated to ")(
                    filename)
                    .Flush();
            } else {
                LogError()("Existing blockchain data directory is obsolete and "
                           "must be purged")
                    .Flush();
                fs::remove_all(base);
            }
        } else {
            LogVerbose()("Initializing new blockchain data directory").Flush();
        }

        if (false == legacy.BuildFolderPath(output)) {
            throw std::runtime_error("Failed to construct path");
        }

        [[maybe_unused]] const auto _ = std::ofstream{version.string()};

        return output;
    }
    static auto siphash_key(storage::lmdb::Database& db) noexcept -> SiphashKey
    {
        auto configured = siphash_key_configured(db);

        if (configured.has_value()) { return configured.value(); }

        auto output = space(crypto_shorthash_KEYBYTES);
        ::crypto_shorthash_keygen(
            reinterpret_cast<unsigned char*>(output.data()));
        const auto saved =
            db.Store(Table::Config, tsv(Key::SiphashKey), reader(output));

        OT_ASSERT(saved.first);

        return output;
    }
    static auto siphash_key_configured(storage::lmdb::Database& db) noexcept
        -> std::optional<SiphashKey>
    {
        if (false == db.Exists(Table::Config, tsv(Key::SiphashKey))) {
            return std::nullopt;
        }

        auto output = space(crypto_shorthash_KEYBYTES);
        auto cb = [&output](const auto in) {
            if (output.size() != in.size()) { return; }

            std::memcpy(output.data(), in.data(), in.size());
        };

        if (false == db.Load(Table::Config, tsv(Key::SiphashKey), cb)) {
            return std::nullopt;
        }

        return output;
    }

    auto AllocateStorageFolder(const fs::path& dir) const noexcept -> fs::path
    {
        return init_folder(legacy_, blockchain_path_, dir);
    }

    Imp(const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::Legacy& legacy,
        const fs::path& dataFolder,
        const Options& args) noexcept(false)
        : api_(api)
        , legacy_(legacy)
        , blockchain_path_(init_storage_path(legacy, dataFolder))
        , common_path_(init_folder(legacy, blockchain_path_, "common"))
        , blocks_path_(init_folder(legacy, common_path_, "blocks"))
        , lmdb_(
              table_names_,
              common_path_,
              [] {
                  auto output = storage::lmdb::TablesToInit{
                      {Table::BlockHeaders, 0},
                      {Table::PeerDetails, 0},
                      {Table::PeerChainIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::PeerProtocolIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::PeerServiceIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::PeerNetworkIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::PeerConnectedIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::FilterHeadersBasic, 0},
                      {Table::FilterHeadersBCH, 0},
                      {Table::FilterHeadersOpentxs, 0},
                      {Table::Config, MDB_INTEGERKEY},
                      {Table::BlockIndex, 0},
                      {Table::Enabled, MDB_INTEGERKEY},
                      {Table::SyncTips, MDB_INTEGERKEY},
                      {Table::ConfigMulti, MDB_DUPSORT | MDB_INTEGERKEY},
                      {Table::FilterIndexBasic, 0},
                      {Table::FilterIndexBCH, 0},
                      {Table::FilterIndexES, 0},
                      {Table::TransactionIndex, 0},
                  };

                  for (const auto& [table, name] : SyncTables()) {
                      output.emplace_back(table, MDB_INTEGERKEY);
                  }

                  return output;
              }(),
              0,
              [&] {
                  auto deleted = UnallocatedVector<Table>{};
                  deleted.emplace_back(Table::FiltersBasicDeleted);
                  deleted.emplace_back(Table::FiltersBCHDeleted);
                  deleted.emplace_back(Table::FiltersOpentxsDeleted);
                  deleted.emplace_back(Table::HeaderIndexDeleted);

                  return deleted.size();
              }())
        , bulk_(lmdb_, blocks_path_)
        , siphash_key_(siphash_key(lmdb_))
        , headers_(lmdb_)
        , peers_(api_, lmdb_)
        , filters_(api_, lmdb_, bulk_)
        , blocks_(lmdb_, bulk_)
        , sync_(api_, lmdb_, blocks_path_)
        , wallet_(api_, blockchain, lmdb_, bulk_)
        , config_(api_, lmdb_)
    {
        OT_ASSERT(crypto_shorthash_KEYBYTES == siphash_key_.size());

        static_assert(sizeof(ElementHash) == crypto_shorthash_BYTES);
    }
};

const storage::lmdb::TableNames Database::Imp::table_names_ = [] {
    auto output = storage::lmdb::TableNames{
        {Table::BlockHeaders, "block_headers"},
        {Table::PeerDetails, "peers"},
        {Table::PeerChainIndex, "peer_chain_index"},
        {Table::PeerProtocolIndex, "peer_protocol_index"},
        {Table::PeerServiceIndex, "peer_service_index"},
        {Table::PeerNetworkIndex, "peer_network_index"},
        {Table::PeerConnectedIndex, "peer_connected_index"},
        {Table::FiltersBasicDeleted, "block_filters_basic"},
        {Table::FiltersBCHDeleted, "block_filters_bch"},
        {Table::FiltersOpentxsDeleted, "block_filters_opentxs"},
        {Table::FilterHeadersBasic, "block_filter_headers_basic"},
        {Table::FilterHeadersBCH, "block_filter_headers_bch"},
        {Table::FilterHeadersOpentxs, "block_filter_headers_opentxs"},
        {Table::Config, "config"},
        {Table::BlockIndex, "blocks"},
        {Table::Enabled, "enabled_chains_2"},
        {Table::SyncTips, "sync_tips"},
        {Table::ConfigMulti, "config_multiple_values"},
        {Table::HeaderIndexDeleted, "block_headers_2"},
        {Table::FilterIndexBasic, "block_filters_basic_2"},
        {Table::FilterIndexBCH, "block_filters_bch_2"},
        {Table::FilterIndexES, "block_filters_opentxs_2"},
        {Table::TransactionIndex, "transactions"},
    };

    for (const auto& [table, name] : SyncTables()) {
        output.emplace(table, name);
    }

    return output;
}();

Database::Database(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    const api::Legacy& legacy,
    const fs::path& dataFolder,
    const Options& args) noexcept(false)
    : imp_(std::make_unique<Imp>(api, blockchain, legacy, dataFolder, args))
{
    OT_ASSERT(imp_);
}

Database::Database(Database&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Database::AddOrUpdate(network::blockchain::Address address) const noexcept
    -> bool
{
    return imp_->peers_.Insert(std::move(address));
}

auto Database::AllocateStorageFolder(
    const UnallocatedCString& dir) const noexcept -> UnallocatedCString
{
    return imp_->AllocateStorageFolder(dir).string();
}

auto Database::AssociateTransaction(
    const block::TransactionHash& txid,
    const ElementHashes& patterns) const noexcept -> bool
{
    return imp_->wallet_.AssociateTransaction(txid, patterns);
}

auto Database::AddSyncServer(std::string_view endpoint) const noexcept -> bool
{
    return imp_->config_.AddSyncServer(endpoint);
}

auto Database::BlockHeaderExists(const block::Hash& hash) const noexcept -> bool
{
    return imp_->headers_.Exists(hash);
}

auto Database::BlockExists(const block::Hash& block) const noexcept -> bool
{
    return imp_->blocks_.Exists(block);
}

auto Database::BlockForget(const block::Hash& block) const noexcept -> bool
{
    return imp_->blocks_.Forget(block);
}

auto Database::BlockLoad(
    blockchain::Type chain,
    const std::span<const block::Hash> hashes,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Vector<storage::file::Position>
{
    return imp_->blocks_.Load(chain, hashes, alloc, monotonic);
}

auto Database::BlockStore(
    const block::Hash& id,
    const ReadView bytes,
    alloc::Default monotonic) const noexcept -> storage::file::Position
{
    return imp_->blocks_.Store(id, bytes, monotonic);
}

auto Database::Confirm(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) const noexcept -> void
{
    imp_->peers_.Confirm(chain, id);
}

auto Database::DeleteSyncServer(std::string_view endpoint) const noexcept
    -> bool
{
    return imp_->config_.DeleteSyncServer(endpoint);
}

auto Database::Disable(const blockchain::Type type) const noexcept -> bool
{
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};
    const auto value = Space{false_byte_};

    return imp_->lmdb_.Store(Enabled, key, reader(value)).first;
}

auto Database::Enable(const blockchain::Type type, std::string_view seednode)
    const noexcept -> bool
{
    static_assert(sizeof(true_byte_) == 1_uz);

    const auto key = std::size_t{static_cast<std::uint32_t>(type)};
    const auto value = [&] {
        auto output = space(sizeof(true_byte_) + seednode.size());
        output.at(0) = true_byte_;
        auto* i = std::next(output.data(), sizeof(true_byte_));
        copy(seednode, preallocated(seednode.size(), i));

        return output;
    }();

    return imp_->lmdb_.Store(Enabled, key, reader(value)).first;
}

auto Database::Fail(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) const noexcept -> void
{
    imp_->peers_.Fail(chain, id);
}

auto Database::Find(
    const blockchain::Type chain,
    const Protocol protocol,
    const Set<Transport>& onNetworks,
    const Set<Service>& withServices,
    const Set<network::blockchain::AddressID>& exclude) const noexcept
    -> network::blockchain::Address
{
    return imp_->peers_.Find(
        chain, protocol, onNetworks, withServices, exclude);
}

auto Database::GetSyncServers(alloc::Default alloc) const noexcept -> Endpoints
{
    return imp_->config_.GetSyncServers(alloc);
}

auto Database::Good(
    const blockchain::Type chain,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept
    -> Vector<network::blockchain::Address>
{
    return imp_->peers_.Good(chain, alloc, monotonic);
}

auto Database::HashKey() const noexcept -> ReadView
{
    return reader(imp_->siphash_key_);
}

auto Database::HaveFilter(const cfilter::Type type, const ReadView blockHash)
    const noexcept -> bool
{
    return imp_->filters_.HaveCfilter(type, blockHash);
}

auto Database::HaveFilterHeader(
    const cfilter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    return imp_->filters_.HaveCfheader(type, blockHash);
}

auto Database::Import(Vector<network::blockchain::Address> peers) const noexcept
    -> bool
{
    return imp_->peers_.Import(std::move(peers));
}

auto Database::LoadBlockHeader(const block::Hash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    return imp_->headers_.Load(hash);
}

auto Database::LoadFilter(
    const cfilter::Type type,
    const ReadView blockHash,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> opentxs::blockchain::GCS
{
    return imp_->filters_.LoadCfilter(type, blockHash, alloc, monotonic);
}

auto Database::LoadFilters(
    const cfilter::Type type,
    const Vector<block::Hash>& blocks,
    alloc::Default monotonic) const noexcept -> Vector<GCS>
{
    return imp_->filters_.LoadCfilters(type, blocks, monotonic);
}

auto Database::LoadFilterHash(
    const cfilter::Type type,
    const ReadView blockHash,
    Writer&& filterHash) const noexcept -> bool
{
    return imp_->filters_.LoadCfilterHash(
        type, blockHash, std::move(filterHash));
}

auto Database::LoadFilterHeader(
    const cfilter::Type type,
    const ReadView blockHash,
    Writer&& header) const noexcept -> bool
{
    return imp_->filters_.LoadCfheader(type, blockHash, std::move(header));
}

auto Database::LoadTransaction(
    const block::TransactionHash& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> block::Transaction
{
    return imp_->wallet_.LoadTransaction(txid, alloc, monotonic);
}

auto Database::LoadTransaction(
    const block::TransactionHash& txid,
    proto::BlockchainTransaction& out,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> block::Transaction
{
    return imp_->wallet_.LoadTransaction(txid, out, alloc, monotonic);
}

auto Database::LookupContact(const Data& pubkeyHash) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return imp_->wallet_.LookupContact(pubkeyHash);
}

auto Database::LoadSync(
    const blockchain::Type chain,
    const block::Height height,
    opentxs::network::otdht::Data& output) const noexcept -> bool
{
    return imp_->sync_.Load(chain, height, output);
}

auto Database::LookupTransactions(const ElementHash pattern) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return imp_->wallet_.LookupTransactions(pattern);
}

auto Database::LoadEnabledChains() const noexcept
    -> UnallocatedVector<EnabledChain>
{
    auto output = UnallocatedVector<EnabledChain>{};
    const auto cb = [&](const auto key, const auto value) -> bool {
        if (0 == value.size()) { return true; }

        auto chain = Chain{};
        auto data = space(value.size());
        std::memcpy(
            static_cast<void*>(&chain),
            key.data(),
            std::min(key.size(), sizeof(chain)));
        std::memcpy(
            static_cast<void*>(data.data()), value.data(), value.size());

        if (true_byte_ == data.front()) {
            auto seed = UnallocatedCString{};
            std::transform(
                std::next(data.begin()),
                data.end(),
                std::back_inserter(seed),
                [](const auto& val) { return static_cast<char>(val); });
            output.emplace_back(chain, std::move(seed));
        }

        return true;
    };
    imp_->lmdb_.Read(Enabled, cb, storage::lmdb::Dir::Forward);

    return output;
}

auto Database::Release(
    const blockchain::Type chain,
    const network::blockchain::AddressID& id) const noexcept -> void
{
    return imp_->peers_.Release(chain, id);
}

auto Database::ReorgSync(
    const blockchain::Type chain,
    const block::Height height) const noexcept -> bool
{
    return imp_->sync_.Reorg(chain, height);
}

auto Database::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    return imp_->headers_.Store(headers);
}

auto Database::StoreFilterHeaders(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers) const noexcept -> bool
{
    return imp_->filters_.StoreCfheaders(type, headers);
}

auto Database::StoreFilters(
    const cfilter::Type type,
    Vector<CFilterParams>& filters,
    alloc::Default monotonic) const noexcept -> bool
{
    return imp_->filters_.StoreCfilters(type, filters, monotonic);
}

auto Database::StoreFilters(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers,
    const Vector<CFilterParams>& filters,
    alloc::Default monotonic) const noexcept -> bool
{
    return imp_->filters_.StoreCfilters(type, headers, filters, monotonic);
}

auto Database::StoreSync(
    const opentxs::network::otdht::SyncData& items,
    Chain chain) const noexcept -> bool
{
    return imp_->sync_.Store(items, chain);
}

auto Database::StoreTransaction(const block::Transaction& tx) const noexcept
    -> bool
{
    return imp_->wallet_.StoreTransaction(tx);
}

auto Database::StoreTransaction(
    const block::Transaction& tx,
    proto::BlockchainTransaction& out) const noexcept -> bool
{
    return imp_->wallet_.StoreTransaction(tx, out);
}

auto Database::SyncTip(const blockchain::Type chain) const noexcept
    -> block::Height
{
    return imp_->sync_.Tip(chain);
}

auto Database::UpdateContact(const Contact& contact) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return imp_->wallet_.UpdateContact(contact);
}

auto Database::UpdateMergedContact(const Contact& parent, const Contact& child)
    const noexcept -> UnallocatedVector<block::TransactionHash>
{
    return imp_->wallet_.UpdateMergedContact(parent, child);
}

Database::~Database() = default;
}  // namespace opentxs::blockchain::database::common
