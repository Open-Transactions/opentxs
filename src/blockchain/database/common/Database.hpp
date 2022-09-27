// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

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
namespace bitcoin
{
namespace block
{
class Block;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Header;
}  // namespace block

class GCS;
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace otdht
{
class Block;
class Data;
}  // namespace otdht
}  // namespace network

namespace proto
{
class BlockchainBlockHeader;
class BlockchainTransaction;
}  // namespace proto

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
        deleted = 0,
        NextBlockAddress = 1,
        SiphashKey = 2,
        NextSyncAddress = 3,
        SyncServerEndpoint = 4,
    };

    using BlockHash = block::Hash;
    using PatternID = blockchain::PatternID;
    using Txid = block::Txid;
    using pTxid = block::pTxid;
    using Chain = blockchain::Type;
    using EnabledChain = std::pair<Chain, UnallocatedCString>;
    using Height = block::Height;
    using Endpoints = Vector<CString>;

    auto AddOrUpdate(Address_p address) const noexcept -> bool;
    auto AddSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto AllocateStorageFolder(const UnallocatedCString& dir) const noexcept
        -> UnallocatedCString;
    auto AssociateTransaction(
        const Txid& txid,
        const UnallocatedVector<PatternID>& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool;
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockForget(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> ReadView;
    auto BlockStore(const block::Block& block) const noexcept -> bool;
    auto DeleteSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type, std::string_view seednode) const noexcept
        -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept -> Address_p;
    auto GetSyncServers(alloc::Default alloc) const noexcept -> Endpoints;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(UnallocatedVector<Address_p> peers) const noexcept -> bool;
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> UnallocatedVector<EnabledChain>;
    auto LoadFilter(
        const cfilter::Type type,
        const ReadView blockHash,
        alloc::Default alloc) const noexcept -> GCS;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks) const noexcept -> Vector<GCS>;
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
        opentxs::network::otdht::Data& output) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LoadTransaction(const ReadView txid, proto::BlockchainTransaction& out)
        const noexcept -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> UnallocatedVector<pTxid>;
    auto ReorgSync(const Chain chain, const Height height) const noexcept
        -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers) const noexcept -> bool;
    auto StoreFilters(const cfilter::Type type, Vector<CFilterParams>& filters)
        const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters) const noexcept -> bool;
    auto StoreSync(const opentxs::network::otdht::SyncData& items, Chain chain)
        const noexcept -> bool;
    auto StoreTransaction(const bitcoin::block::Transaction& tx) const noexcept
        -> bool;
    auto StoreTransaction(
        const bitcoin::block::Transaction& tx,
        proto::BlockchainTransaction& out) const noexcept -> bool;
    auto SyncTip(const Chain chain) const noexcept -> Height;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<pTxid>;

    Database(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::Legacy& legacy,
        const std::filesystem::path& dataFolder,
        const Options& args) noexcept(false);
    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&& rhs) noexcept;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;

    ~Database();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::common
