// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/common/Database.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
#include <sodium.h>
}

#include <boost/filesystem.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <type_traits>
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
#include "internal/util/TSV.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "util/LMDB.hpp"

constexpr auto false_byte_ = std::byte{0x0};
constexpr auto true_byte_ = std::byte{0x1};

namespace opentxs::blockchain::database::common
{
struct Database::Imp {
    using SiphashKey = Space;

    static const storage::lmdb::TableNames table_names_;

    const api::Session& api_;
    const api::Legacy& legacy_;
    const OTString blockchain_path_;
    const OTString common_path_;
    const OTString blocks_path_;
    storage::lmdb::LMDB lmdb_;
    Bulk bulk_;
    const BlockStorage block_policy_;
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
    static auto block_storage_level(
        const Options& args,
        storage::lmdb::LMDB& lmdb) noexcept -> BlockStorage
    {
        if (false == block_storage_enabled()) { return BlockStorage::None; }

        auto output = block_storage_level_default();
        const auto arg = block_storage_level_arg(args);

        if (arg.has_value()) { output = arg.value(); }

        const auto db = block_storage_level_configured(lmdb);

        if (db.has_value()) { output = std::max(output, db.value()); }

        if ((false == db.has_value()) || (output != db.value())) {
            lmdb.Store(
                Table::Config, tsv(Key::BlockStoragePolicy), tsv(output));
        }

        return output;
    }
    static auto block_storage_level_arg(const Options& options) noexcept
        -> std::optional<BlockStorage>
    {
        switch (options.BlockchainStorageLevel()) {
            case 2: {
                return BlockStorage::All;
            }
            case 1: {
                return BlockStorage::Cache;
            }
            default: {
                return BlockStorage::None;
            }
        }
    }
    static auto block_storage_level_configured(storage::lmdb::LMDB& db) noexcept
        -> std::optional<BlockStorage>
    {
        if (false == db.Exists(Table::Config, tsv(Key::BlockStoragePolicy))) {
            return std::nullopt;
        }

        auto output{BlockStorage::None};
        auto cb = [&output](const auto in) {
            if (sizeof(output) != in.size()) { return; }

            std::memcpy(&output, in.data(), in.size());
        };

        if (false == db.Load(Table::Config, tsv(Key::BlockStoragePolicy), cb)) {
            return std::nullopt;
        }

        return output;
    }
    static auto block_storage_level_default() noexcept -> BlockStorage
    {
        if (2 == default_storage_level_) {

            return BlockStorage::All;
        } else if (1 == default_storage_level_) {

            return BlockStorage::Cache;
        } else {

            return BlockStorage::None;
        }
    }
    static auto init_folder(
        const api::Legacy& legacy,
        const String& parent,
        const String& child) noexcept(false) -> OTString
    {
        auto output = String::Factory();

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
        const UnallocatedCString& dataFolder) noexcept(false) -> OTString
    {
        auto output = String::Factory();

        if (false == legacy.AppendFolder(
                         output,
                         String::Factory(dataFolder),
                         String::Factory("blockchain"))) {
            throw std::runtime_error("Failed to calculate path");
        }

        namespace fs = boost::filesystem;

        constexpr auto version1{"version.1"};
        const auto base = fs::path{output->Get()};
        const auto v1 = base / fs::path{version1};
        const auto haveBase = [&] {
            try {

                return fs::exists(base);
            } catch (...) {

                return false;
            }
        }();
        const auto haveV1 = [&] {
            try {

                return fs::exists(v1);
            } catch (...) {

                return false;
            }
        }();

        if (haveBase) {
            if (haveV1) {
                LogVerbose()(
                    "Existing blockchain data directory already updated to v1")
                    .Flush();
            } else {
                LogError()("Existing blockchain data directory is v0 and must "
                           "be purged")
                    .Flush();
                fs::remove_all(base);
            }
        } else {
            LogVerbose()("Initializing new blockchain data directory").Flush();
        }

        if (false == legacy.BuildFolderPath(output)) {
            throw std::runtime_error("Failed to construct path");
        }

        std::ofstream{v1.string()};

        return output;
    }
    static auto siphash_key(storage::lmdb::LMDB& db) noexcept -> SiphashKey
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
    static auto siphash_key_configured(storage::lmdb::LMDB& db) noexcept
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

        return std::move(output);
    }

    auto AllocateStorageFolder(const UnallocatedCString& dir) const noexcept
        -> UnallocatedCString
    {
        return init_folder(legacy_, blockchain_path_, String::Factory(dir))
            ->Get();
    }

    Imp(const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::Legacy& legacy,
        const UnallocatedCString& dataFolder,
        const Options& args) noexcept(false)
        : api_(api)
        , legacy_(legacy)
        , blockchain_path_(init_storage_path(legacy, dataFolder))
        , common_path_(
              init_folder(legacy, blockchain_path_, String::Factory("common")))
        , blocks_path_(
              init_folder(legacy, common_path_, String::Factory("blocks")))
        , lmdb_(
              table_names_,
              common_path_->Get(),
              [] {
                  auto output = storage::lmdb::TablesToInit{
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
                      {Table::HeaderIndex, 0},
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
                  deleted.emplace_back(Table::BlockHeadersDeleted);
                  deleted.emplace_back(Table::FiltersBasicDeleted);
                  deleted.emplace_back(Table::FiltersBCHDeleted);
                  deleted.emplace_back(Table::FiltersOpentxsDeleted);

                  return deleted.size();
              }())
        , bulk_(lmdb_, blocks_path_->Get())
        , block_policy_(block_storage_level(args, lmdb_))
        , siphash_key_(siphash_key(lmdb_))
        , headers_(lmdb_, bulk_)
        , peers_(api_, lmdb_)
        , filters_(api_, lmdb_, bulk_)
        , blocks_(lmdb_, bulk_)
        , sync_(api_, lmdb_, blocks_path_->Get())
        , wallet_(api_, blockchain, lmdb_, bulk_)
        , config_(api_, lmdb_)
    {
        OT_ASSERT(crypto_shorthash_KEYBYTES == siphash_key_.size());

        static_assert(
            sizeof(opentxs::blockchain::PatternID) == crypto_shorthash_BYTES);
    }
};

const storage::lmdb::TableNames Database::Imp::table_names_ = [] {
    auto output = storage::lmdb::TableNames{
        {Table::BlockHeadersDeleted, "block_headers"},
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
        {Table::HeaderIndex, "block_headers_2"},
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
    const UnallocatedCString& dataFolder,
    const Options& args) noexcept(false)
    : imp_p_(std::make_unique<Imp>(api, blockchain, legacy, dataFolder, args))
    , imp_(*imp_p_)
{
    OT_ASSERT(imp_p_)
}

auto Database::AddOrUpdate(Address_p address) const noexcept -> bool
{
    return imp_.peers_.Insert(std::move(address));
}

auto Database::AllocateStorageFolder(
    const UnallocatedCString& dir) const noexcept -> UnallocatedCString
{
    return imp_.AllocateStorageFolder(dir);
}

auto Database::AssociateTransaction(
    const Txid& txid,
    const UnallocatedVector<PatternID>& patterns) const noexcept -> bool
{
    return imp_.wallet_.AssociateTransaction(txid, patterns);
}

auto Database::AddSyncServer(const UnallocatedCString& endpoint) const noexcept
    -> bool
{
    return imp_.config_.AddSyncServer(endpoint);
}

auto Database::BlockHeaderExists(const BlockHash& hash) const noexcept -> bool
{
    return imp_.headers_.Exists(hash);
}

auto Database::BlockExists(const BlockHash& block) const noexcept -> bool
{
    return imp_.blocks_.Exists(block);
}

auto Database::BlockLoad(const BlockHash& block) const noexcept -> BlockReader
{
    return imp_.blocks_.Load(block);
}

auto Database::BlockPolicy() const noexcept -> BlockStorage
{
    return imp_.block_policy_;
}

auto Database::BlockStore(const BlockHash& block, const std::size_t bytes)
    const noexcept -> BlockWriter
{
    return imp_.blocks_.Store(block, bytes);
}

auto Database::DeleteSyncServer(
    const UnallocatedCString& endpoint) const noexcept -> bool
{
    return imp_.config_.DeleteSyncServer(endpoint);
}

auto Database::Disable(const Chain type) const noexcept -> bool
{
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};
    const auto value = Space{false_byte_};

    return imp_.lmdb_.Store(Enabled, key, reader(value)).first;
}

auto Database::Enable(const Chain type, const UnallocatedCString& seednode)
    const noexcept -> bool
{
    static_assert(sizeof(true_byte_) == 1);

    const auto key = std::size_t{static_cast<std::uint32_t>(type)};
    const auto value = [&] {
        auto output = space(sizeof(true_byte_) + seednode.size());
        output.at(0) = true_byte_;
        auto it = std::next(output.data(), sizeof(true_byte_));
        std::memcpy(it, seednode.data(), seednode.size());

        return output;
    }();

    return imp_.lmdb_.Store(Enabled, key, reader(value)).first;
}

auto Database::Find(
    const Chain chain,
    const Protocol protocol,
    const UnallocatedSet<Type> onNetworks,
    const UnallocatedSet<Service> withServices) const noexcept -> Address_p
{
    return imp_.peers_.Find(chain, protocol, onNetworks, withServices);
}

auto Database::GetSyncServers() const noexcept -> Endpoints
{
    return imp_.config_.GetSyncServers();
}

auto Database::HashKey() const noexcept -> ReadView
{
    return reader(imp_.siphash_key_);
}

auto Database::HaveFilter(const cfilter::Type type, const ReadView blockHash)
    const noexcept -> bool
{
    return imp_.filters_.HaveFilter(type, blockHash);
}

auto Database::HaveFilterHeader(
    const cfilter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    return imp_.filters_.HaveFilterHeader(type, blockHash);
}

auto Database::Import(UnallocatedVector<Address_p> peers) const noexcept -> bool
{
    return imp_.peers_.Import(std::move(peers));
}

auto Database::LoadBlockHeader(const BlockHash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    return imp_.headers_.Load(hash);
}

auto Database::LoadFilter(const cfilter::Type type, const ReadView blockHash)
    const noexcept -> std::unique_ptr<const opentxs::blockchain::GCS>
{
    return imp_.filters_.LoadFilter(type, blockHash);
}

auto Database::LoadFilterHash(
    const cfilter::Type type,
    const ReadView blockHash,
    const AllocateOutput filterHash) const noexcept -> bool
{
    return imp_.filters_.LoadFilterHash(type, blockHash, filterHash);
}

auto Database::LoadFilterHeader(
    const cfilter::Type type,
    const ReadView blockHash,
    const AllocateOutput header) const noexcept -> bool
{
    return imp_.filters_.LoadFilterHeader(type, blockHash, header);
}

auto Database::LoadTransaction(const ReadView txid) const noexcept
    -> std::unique_ptr<block::bitcoin::Transaction>
{
    return imp_.wallet_.LoadTransaction(txid);
}

auto Database::LookupContact(const Data& pubkeyHash) const noexcept
    -> UnallocatedSet<OTIdentifier>
{
    return imp_.wallet_.LookupContact(pubkeyHash);
}

auto Database::LoadSync(
    const Chain chain,
    const Height height,
    opentxs::network::p2p::Data& output) const noexcept -> bool
{
    return imp_.sync_.Load(chain, height, output);
}

auto Database::LookupTransactions(const PatternID pattern) const noexcept
    -> UnallocatedVector<pTxid>
{
    return imp_.wallet_.LookupTransactions(pattern);
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
                [](const auto& value) { return static_cast<char>(value); });
            output.emplace_back(chain, std::move(seed));
        }

        return true;
    };
    imp_.lmdb_.Read(Enabled, cb, storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Database::ReorgSync(const Chain chain, const Height height) const noexcept
    -> bool
{
    return imp_.sync_.Reorg(chain, height);
}

auto Database::StoreBlockHeader(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    return imp_.headers_.Store(header);
}

auto Database::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    return imp_.headers_.Store(headers);
}

auto Database::StoreFilterHeaders(
    const cfilter::Type type,
    const UnallocatedVector<FilterHeader>& headers) const noexcept -> bool
{
    return imp_.filters_.StoreFilterHeaders(type, headers);
}

auto Database::StoreFilters(
    const cfilter::Type type,
    UnallocatedVector<FilterData>& filters) const noexcept -> bool
{
    return imp_.filters_.StoreFilters(type, filters);
}

auto Database::StoreFilters(
    const cfilter::Type type,
    const UnallocatedVector<FilterHeader>& headers,
    const UnallocatedVector<FilterData>& filters) const noexcept -> bool
{
    return imp_.filters_.StoreFilters(type, headers, filters);
}

auto Database::StoreSync(const Chain chain, const SyncItems& items)
    const noexcept -> bool
{
    return imp_.sync_.Store(chain, items);
}

auto Database::StoreTransaction(
    const block::bitcoin::Transaction& tx) const noexcept -> bool
{
    return imp_.wallet_.StoreTransaction(tx);
}

auto Database::SyncTip(const Chain chain) const noexcept -> Height
{
    return imp_.sync_.Tip(chain);
}

auto Database::UpdateContact(const Contact& contact) const noexcept
    -> UnallocatedVector<pTxid>
{
    return imp_.wallet_.UpdateContact(contact);
}

auto Database::UpdateMergedContact(const Contact& parent, const Contact& child)
    const noexcept -> UnallocatedVector<pTxid>
{
    return imp_.wallet_.UpdateMergedContact(parent, child);
}

Database::~Database() = default;
}  // namespace opentxs::blockchain::database::common
