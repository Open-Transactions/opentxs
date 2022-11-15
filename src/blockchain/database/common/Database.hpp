// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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

namespace p2p
{
class Address;
}  // namespace p2p

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

    auto AddOrUpdate(p2p::Address address) const noexcept -> bool;
    auto AddSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto AllocateStorageFolder(const UnallocatedCString& dir) const noexcept
        -> UnallocatedCString;
    auto AssociateTransaction(
        const block::Txid& txid,
        const ElementHashes& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const block::Hash& hash) const noexcept -> bool;
    auto BlockExists(const block::Hash& block) const noexcept -> bool;
    auto BlockForget(const block::Hash& block) const noexcept -> bool;
    auto BlockLoad(const block::Hash& block) const noexcept -> ReadView;
    auto BlockStore(const block::Block& block) const noexcept -> bool;
    auto DeleteSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto Disable(const blockchain::Type type) const noexcept -> bool;
    auto Enable(const blockchain::Type type, std::string_view seednode)
        const noexcept -> bool;
    auto Find(
        const blockchain::Type chain,
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept
        -> p2p::Address;
    auto GetSyncServers(alloc::Default alloc) const noexcept -> Endpoints;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(UnallocatedVector<p2p::Address> peers) const noexcept -> bool;
    auto LoadBlockHeader(const block::Hash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> UnallocatedVector<EnabledChain>;
    auto LoadFilter(
        const cfilter::Type type,
        const ReadView blockHash,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> GCS;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks,
        alloc::Default monotonic) const noexcept -> Vector<GCS>;
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
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LoadTransaction(const ReadView txid, proto::BlockchainTransaction& out)
        const noexcept -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto LookupTransactions(const ElementHash pattern) const noexcept
        -> UnallocatedVector<block::pTxid>;
    auto ReorgSync(const blockchain::Type chain, const block::Height height)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams>& filters,
        alloc::Default monotonic) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        alloc::Default monotonic) const noexcept -> bool;
    auto StoreSync(const opentxs::network::otdht::SyncData& items, Chain chain)
        const noexcept -> bool;
    auto StoreTransaction(const bitcoin::block::Transaction& tx) const noexcept
        -> bool;
    auto StoreTransaction(
        const bitcoin::block::Transaction& tx,
        proto::BlockchainTransaction& out) const noexcept -> bool;
    auto SyncTip(const blockchain::Type chain) const noexcept -> block::Height;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<block::pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<block::pTxid>;

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
