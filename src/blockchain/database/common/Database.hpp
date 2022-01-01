// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/util/Bytes.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Legacy;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Transaction;
}  // namespace bitcoin

class Header;
}  // namespace block

class GCS;
}  // namespace blockchain

namespace contact
{
class Contact;
}  // namespace contact

namespace network
{
namespace p2p
{
class Block;
class Data;
}  // namespace p2p
}  // namespace network

class Data;
class Options;
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Database final
{
public:
    static const int default_storage_level_;
    static const int storage_enabled_;

    enum class Key : std::size_t {
        BlockStoragePolicy = 0,
        NextBlockAddress = 1,
        SiphashKey = 2,
        NextSyncAddress = 3,
        SyncServerEndpoint = 4,
    };

    using BlockHash = opentxs::blockchain::block::Hash;
    using PatternID = opentxs::blockchain::PatternID;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using Chain = opentxs::blockchain::Type;
    using EnabledChain = std::pair<Chain, std::string>;
    using Height = opentxs::blockchain::block::Height;
    using SyncItems = std::pmr::vector<opentxs::network::p2p::Block>;
    using Endpoints = std::pmr::vector<std::string>;

    auto AddOrUpdate(Address_p address) const noexcept -> bool;
    auto AddSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto AllocateStorageFolder(const std::string& dir) const noexcept
        -> std::string;
    auto AssociateTransaction(
        const Txid& txid,
        const std::pmr::vector<PatternID>& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool;
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> BlockReader;
    auto BlockPolicy() const noexcept -> BlockStorage;
    auto BlockStore(const BlockHash& block, const std::size_t bytes)
        const noexcept -> BlockWriter;
    auto DeleteSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type, const std::string& seednode) const noexcept
        -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const std::pmr::set<Type> onNetworks,
        const std::pmr::set<Service> withServices) const noexcept -> Address_p;
    auto GetSyncServers() const noexcept -> Endpoints;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const filter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const filter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(std::pmr::vector<Address_p> peers) const noexcept -> bool;
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> std::pmr::vector<EnabledChain>;
    auto LoadFilter(const filter::Type type, const ReadView blockHash)
        const noexcept -> std::unique_ptr<const opentxs::blockchain::GCS>;
    auto LoadFilterHash(
        const filter::Type type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const filter::Type type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto LoadSync(
        const Chain chain,
        const Height height,
        opentxs::network::p2p::Data& output) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::pmr::set<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> std::pmr::vector<pTxid>;
    auto ReorgSync(const Chain chain, const Height height) const noexcept
        -> bool;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const filter::Type type,
        const std::pmr::vector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(
        const filter::Type type,
        std::pmr::vector<FilterData>& filters) const noexcept -> bool;
    auto StoreFilters(
        const filter::Type type,
        const std::pmr::vector<FilterHeader>& headers,
        const std::pmr::vector<FilterData>& filters) const noexcept -> bool;
    auto StoreSync(const Chain chain, const SyncItems& items) const noexcept
        -> bool;
    auto StoreTransaction(const block::bitcoin::Transaction& tx) const noexcept
        -> bool;
    auto SyncTip(const Chain chain) const noexcept -> Height;
    auto UpdateContact(const contact::Contact& contact) const noexcept
        -> std::pmr::vector<pTxid>;
    auto UpdateMergedContact(
        const contact::Contact& parent,
        const contact::Contact& child) const noexcept
        -> std::pmr::vector<pTxid>;

    Database(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const Options& args) noexcept(false);

    ~Database();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_p_;
    Imp& imp_;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;
};
}  // namespace opentxs::blockchain::database::common
