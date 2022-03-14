// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "serialization/protobuf/BlockchainTransaction.pb.h"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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

namespace network
{
namespace p2p
{
class Block;
class Data;
}  // namespace p2p
}  // namespace network

class Contact;
class Data;
class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    using EnabledChain = std::pair<Chain, UnallocatedCString>;
    using Height = opentxs::blockchain::block::Height;
    using SyncItems = UnallocatedVector<opentxs::network::p2p::Block>;
    using Endpoints = UnallocatedVector<UnallocatedCString>;

    auto AddOrUpdate(Address_p address) const noexcept -> bool;
    auto AddSyncServer(const UnallocatedCString& endpoint) const noexcept
        -> bool;
    auto AllocateStorageFolder(const UnallocatedCString& dir) const noexcept
        -> UnallocatedCString;
    auto AssociateTransaction(
        const Txid& txid,
        const UnallocatedVector<PatternID>& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool;
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> BlockReader;
    auto BlockPolicy() const noexcept -> BlockStorage;
    auto BlockStore(const BlockHash& block, const std::size_t bytes)
        const noexcept -> BlockWriter;
    auto DeleteSyncServer(const UnallocatedCString& endpoint) const noexcept
        -> bool;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type, const UnallocatedCString& seednode)
        const noexcept -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept -> Address_p;
    auto GetSyncServers() const noexcept -> Endpoints;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(UnallocatedVector<Address_p> peers) const noexcept -> bool;
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> UnallocatedVector<EnabledChain>;
    auto LoadFilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> std::unique_ptr<const opentxs::blockchain::GCS>;
    auto LoadFilterHash(
        const cfilter::Type type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const cfilter::Type type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto LoadSync(
        const Chain chain,
        const Height height,
        opentxs::network::p2p::Data& output) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> UnallocatedVector<pTxid>;
    auto ReorgSync(const Chain chain, const Height height) const noexcept
        -> bool;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const cfilter::Type type,
        const UnallocatedVector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        UnallocatedVector<FilterData>& filters) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const UnallocatedVector<FilterHeader>& headers,
        const UnallocatedVector<FilterData>& filters) const noexcept -> bool;
    auto StoreSync(const Chain chain, const SyncItems& items) const noexcept
        -> bool;
    auto StoreTransaction(const block::bitcoin::Transaction& tx) const noexcept
        -> bool;
    auto SyncTip(const Chain chain) const noexcept -> Height;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<pTxid>;

    Database(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::Legacy& legacy,
        const UnallocatedCString& dataFolder,
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
