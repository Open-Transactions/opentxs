// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace internal
{
class Paths;
}  // namespace internal

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace cfilter
{
class GCS;
}  // namespace cfilter
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain

namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network

namespace protobuf
{
class BlockchainBlockHeader;
class BlockchainTransaction;
}  // namespace protobuf

class Contact;
class Data;
class Options;
class Writer;
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

    using EnabledChain = std::pair<blockchain::Type, UnallocatedCString>;
    using Endpoints = Vector<CString>;

    auto AddOrUpdate(network::blockchain::Address address) const noexcept
        -> bool;
    auto AddSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto AllocateStorageFolder(const UnallocatedCString& dir) const noexcept
        -> UnallocatedCString;
    auto AssociateTransaction(
        const block::TransactionHash& txid,
        const ElementHashes& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const block::Hash& hash) const noexcept -> bool;
    auto BlockExists(const block::Hash& block) const noexcept -> bool;
    auto BlockForget(const block::Hash& block) const noexcept -> bool;
    auto BlockLoad(
        blockchain::Type chain,
        const std::span<const block::Hash> hashes,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Vector<ReadView>;
    auto BlockStore(
        const block::Hash& id,
        const ReadView bytes,
        alloc::Default monotonic) const noexcept -> ReadView;
    auto Confirm(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) const noexcept -> void;
    auto DeleteSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto Disable(const blockchain::Type type) const noexcept -> bool;
    auto Enable(const blockchain::Type type, std::string_view seednode)
        const noexcept -> bool;
    auto Fail(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) const noexcept -> void;
    auto Find(
        const blockchain::Type chain,
        const Protocol protocol,
        const Set<Transport>& onNetworks,
        const Set<Service>& withServices,
        const Set<network::blockchain::AddressID>& exclude) const noexcept
        -> network::blockchain::Address;
    auto GetSyncServers(alloc::Default alloc) const noexcept -> Endpoints;
    auto Good(
        const blockchain::Type chain,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> Vector<network::blockchain::Address>;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(Vector<network::blockchain::Address> peers) const noexcept
        -> bool;
    auto LoadBlockHeader(const block::Hash& hash) const noexcept(false)
        -> protobuf::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> UnallocatedVector<EnabledChain>;
    auto LoadFilter(
        const cfilter::Type type,
        const ReadView blockHash,
        alloc::Strategy alloc) const noexcept -> cfilter::GCS;
    auto LoadFilters(
        const cfilter::Type type,
        std::span<const block::Hash> blocks,
        alloc::Strategy alloc) const noexcept -> Vector<cfilter::GCS>;
    auto LoadFilterHash(
        const cfilter::Type type,
        const ReadView blockHash,
        Writer&& filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const cfilter::Type type,
        const ReadView blockHash,
        Writer&& header) const noexcept -> bool;
    auto LoadSync(
        const blockchain::Type chain,
        const block::Height height,
        opentxs::network::otdht::Data& output) const noexcept -> bool;
    auto LoadTransaction(
        const block::TransactionHash& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> block::Transaction;
    auto LoadTransaction(
        const block::TransactionHash& txid,
        protobuf::BlockchainTransaction& out,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> block::Transaction;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto LookupTransactions(const ElementHash pattern) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto PeerIsReady() const noexcept -> bool;
    auto Release(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) const noexcept -> void;
    auto ReorgSync(const blockchain::Type chain, const block::Height height)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams>& filters,
        alloc::Strategy alloc) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        alloc::Strategy alloc) const noexcept -> bool;
    auto StoreSync(const opentxs::network::otdht::SyncData& items, Chain chain)
        const noexcept -> bool;
    auto StoreTransaction(const block::Transaction& tx) const noexcept -> bool;
    auto StoreTransaction(
        const block::Transaction& tx,
        protobuf::BlockchainTransaction& out) const noexcept -> bool;
    auto SyncTip(const blockchain::Type chain) const noexcept -> block::Height;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<block::TransactionHash>;

    Database(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const api::internal::Paths& legacy,
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
