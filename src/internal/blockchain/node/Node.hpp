// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#if OT_BLOCKCHAIN
#include "blockchain/DownloadTask.hpp"
#include "internal/blockchain/database/Database.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/core/Core.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Blank.hpp"
#include "util/LMDB.hpp"
#include "util/Work.hpp"

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

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Output;
struct Transaction;
}  // namespace internal

class Block;
class Header;
class Output;
class Transaction;
}  // namespace bitcoin

class Block;
class Header;
class Outpoint;
}  // namespace block

namespace cfilter
{
class Hash;
class Header;
}  // namespace cfilter

namespace internal
{
struct Database;
}  // namespace internal

namespace node
{
namespace internal
{
class HeaderOracle;
}  // namespace internal

class HeaderOracle;
class UpdateTransaction;
}  // namespace node

namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal

class Address;
}  // namespace p2p

class GCS;
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace p2p
{
class Block;
class Data;
}  // namespace p2p

namespace zeromq
{
class Frame;
class Message;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto

class Data;
template <typename T>
struct make_blank;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

#if OT_BLOCKCHAIN
namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static auto value(const api::Session&) -> blockchain::block::Height
    {
        return -1;
    }
};
template <>
struct make_blank<blockchain::block::Position> {
    static auto value(const api::Session& api) -> blockchain::block::Position
    {
        return {
            make_blank<blockchain::block::Height>::value(api),
            blockchain::block::Hash{}};
    }
};
}  // namespace opentxs

namespace opentxs::blockchain::node
{
// parent hash, child hash
using ChainSegment = std::pair<block::Hash, block::Hash>;
using UpdatedHeader = UnallocatedMap<
    block::Hash,
    std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = UnallocatedMap<block::Height, block::Hash>;
using Hashes = UnallocatedSet<block::Hash>;
using HashVector = Vector<block::Hash>;
using Segments = UnallocatedSet<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = UnallocatedMultimap<block::Hash, block::Hash>;

using CfheaderJob =
    download::Batch<cfilter::Hash, cfilter::Header, cfilter::Type>;
using CfilterJob = download::Batch<GCS, cfilter::Header, cfilter::Type>;
using BlockJob =
    download::Batch<std::shared_ptr<const block::bitcoin::Block>, int>;

// WARNING update print function if new values are added or removed
enum class BlockOracleJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    request_blocks = OT_ZMQ_INTERNAL_SIGNAL + 0,
    process_block = OT_ZMQ_INTERNAL_SIGNAL + 1,
    start_downloader = OT_ZMQ_INTERNAL_SIGNAL + 2,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(BlockOracleJobs) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::node
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::node::internal
{
#if OT_BLOCKCHAIN
struct BlockDatabase {
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> = 0;
    virtual auto BlockPolicy() const noexcept -> database::BlockStorage = 0;
    virtual auto BlockTip() const noexcept -> block::Position = 0;

    virtual auto BlockStore(const block::Block& block) noexcept -> bool = 0;
    virtual auto SetBlockTip(const block::Position& position) noexcept
        -> bool = 0;

    virtual ~BlockDatabase() = default;
};

struct BlockValidator {
    using BitcoinBlock = block::bitcoin::Block;

    virtual auto Validate(const BitcoinBlock& block) const noexcept -> bool
    {
        return true;
    }

    virtual ~BlockValidator() = default;
};

struct Config {
    bool download_cfilters_{false};
    bool generate_cfilters_{false};
    bool provide_sync_server_{false};
    bool use_sync_server_{false};
    bool disable_wallet_{false};

    auto print() const noexcept -> UnallocatedCString;
};

struct FilterDatabase {
    using CFHeaderParams =
        std::tuple<block::Hash, cfilter::Header, cfilter::Hash>;
    using CFilterParams = std::pair<block::Hash, GCS>;

    virtual auto FilterHeaderTip(const cfilter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto HaveFilter(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool = 0;
    virtual auto HaveFilterHeader(
        const cfilter::Type type,
        const block::Hash& block) const noexcept -> bool = 0;
    virtual auto LoadFilter(
        const cfilter::Type type,
        const ReadView block,
        alloc::Default alloc) const noexcept -> blockchain::GCS = 0;
    virtual auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks) const noexcept -> Vector<GCS> = 0;
    virtual auto LoadFilterHash(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Hash = 0;
    virtual auto LoadFilterHeader(
        const cfilter::Type type,
        const ReadView block) const noexcept -> cfilter::Header = 0;

    virtual auto SetFilterHeaderTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool = 0;
    virtual auto SetFilterTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool = 0;
    virtual auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams> filters) noexcept -> bool = 0;
    virtual auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        const block::Position& tip) noexcept -> bool = 0;
    virtual auto StoreFilterHeaders(
        const cfilter::Type type,
        const ReadView previous,
        const Vector<CFHeaderParams> headers) noexcept -> bool = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle : virtual public node::FilterOracle {
    virtual auto GetFilterJob() const noexcept -> CfilterJob = 0;
    virtual auto GetHeaderJob() const noexcept -> CfheaderJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto LoadFilterOrResetTip(
        const cfilter::Type type,
        const block::Position& position,
        alloc::Default alloc) const noexcept -> GCS = 0;
    virtual auto ProcessBlock(const block::bitcoin::Block& block) const noexcept
        -> bool = 0;
    virtual auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::p2p::Data& data) const noexcept -> void = 0;
    virtual auto Tip(const cfilter::Type type) const noexcept
        -> block::Position = 0;

    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~FilterOracle() override = default;
};

struct HeaderDatabase {
    // Throws std::out_of_range if no block at that position
    virtual auto BestBlock(const block::Height position) const noexcept(false)
        -> block::Hash = 0;
    virtual auto CurrentBest() const noexcept
        -> std::unique_ptr<block::Header> = 0;
    virtual auto CurrentCheckpoint() const noexcept -> block::Position = 0;
    virtual auto DisconnectedHashes() const noexcept -> DisconnectedList = 0;
    virtual auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto HaveCheckpoint() const noexcept -> bool = 0;
    virtual auto HeaderExists(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto IsSibling(const block::Hash& hash) const noexcept -> bool = 0;
    // Throws std::out_of_range if the header does not exist
    virtual auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> = 0;
    virtual auto RecentHashes(alloc::Resource* alloc = alloc::System())
        const noexcept -> HashVector = 0;
    virtual auto SiblingHashes() const noexcept -> Hashes = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    virtual auto ApplyUpdate(const UpdateTransaction& update) noexcept
        -> bool = 0;

    virtual ~HeaderDatabase() = default;
};

struct Mempool {
    virtual auto Dump() const noexcept
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const block::bitcoin::Transaction> = 0;
    virtual auto Submit(ReadView txid) const noexcept -> bool = 0;
    virtual auto Submit(const UnallocatedVector<ReadView>& txids) const noexcept
        -> UnallocatedVector<bool> = 0;
    virtual auto Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
        const noexcept -> void = 0;

    virtual auto Heartbeat() noexcept -> void = 0;

    virtual ~Mempool() = default;
};

struct PeerDatabase {
    using Address = std::unique_ptr<blockchain::p2p::internal::Address>;
    using Protocol = blockchain::p2p::Protocol;
    using Service = blockchain::p2p::Service;
    using Type = blockchain::p2p::Network;

    virtual auto Get(
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept
        -> Address = 0;

    virtual auto AddOrUpdate(Address address) noexcept -> bool = 0;
    virtual auto Import(UnallocatedVector<Address> peers) noexcept -> bool = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        Mempool = value(WorkType::BlockchainMempoolUpdated),
        Register = value(WorkType::AsioRegister),
        Connect = value(WorkType::AsioConnect),
        Disconnect = value(WorkType::AsioDisconnect),
        P2P = value(WorkType::BitcoinP2P),
        Getheaders = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Getblock = OT_ZMQ_INTERNAL_SIGNAL + 1,
        BroadcastTransaction = OT_ZMQ_INTERNAL_SIGNAL + 2,
        BroadcastBlock = OT_ZMQ_INTERNAL_SIGNAL + 3,
        JobAvailableCfheaders = OT_ZMQ_INTERNAL_SIGNAL + 4,
        JobAvailableCfilters = OT_ZMQ_INTERNAL_SIGNAL + 5,
        JobAvailableBlock = OT_ZMQ_INTERNAL_SIGNAL + 6,
        ActivityTimeout = OT_ZMQ_INTERNAL_SIGNAL + 124,
        NeedPing = OT_ZMQ_INTERNAL_SIGNAL + 125,
        Body = OT_ZMQ_INTERNAL_SIGNAL + 126,
        Header = OT_ZMQ_INTERNAL_SIGNAL + 127,
        Heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        Init = OT_ZMQ_INIT_SIGNAL,
        ReceiveMessage = OT_ZMQ_RECEIVE_SIGNAL,
        SendMessage = OT_ZMQ_SEND_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    virtual auto AddIncomingPeer(const int id, std::uintptr_t endpoint)
        const noexcept -> void = 0;
    virtual auto AddPeer(const blockchain::p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto BroadcastBlock(const block::Block& block) const noexcept
        -> bool = 0;
    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Disconnect(const int id) const noexcept -> void = 0;
    virtual auto Endpoint(const Task type) const noexcept
        -> UnallocatedCString = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto GetVerifiedPeerCount() const noexcept -> std::size_t = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto JobReady(const Task type) const noexcept -> void = 0;
    virtual auto Listen(const blockchain::p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto LookupIncomingSocket(const int id) const noexcept(false)
        -> opentxs::network::asio::Socket = 0;
    virtual auto PeerTarget() const noexcept -> std::size_t = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const UnallocatedVector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto RequestHeaders() const noexcept -> bool = 0;
    virtual auto VerifyPeer(const int id, const UnallocatedCString& address)
        const noexcept -> void = 0;

    virtual auto init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~PeerManager() = default;
};

struct Network : virtual public node::Manager {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        SyncReply = value(WorkType::P2PBlockchainSyncReply),
        SyncNewBlock = value(WorkType::P2PBlockchainNewBlock),
        SubmitBlockHeader = OT_ZMQ_INTERNAL_SIGNAL + 0,
        SubmitBlock = OT_ZMQ_INTERNAL_SIGNAL + 2,
        Heartbeat = OT_ZMQ_INTERNAL_SIGNAL + 3,
        SendToAddress = OT_ZMQ_INTERNAL_SIGNAL + 4,
        SendToPaymentCode = OT_ZMQ_INTERNAL_SIGNAL + 5,
        StartWallet = OT_ZMQ_INTERNAL_SIGNAL + 6,
        FilterUpdate = OT_ZMQ_NEW_FILTER_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx,
        const bool pushtx = false) const noexcept -> bool = 0;
    virtual auto Chain() const noexcept -> Type = 0;
    // amount represents satoshis per 1000 bytes
    virtual auto FeeRate() const noexcept -> Amount = 0;
    auto FilterOracle() const noexcept -> const node::FilterOracle& final
    {
        return FilterOracleInternal();
    }
    virtual auto FilterOracleInternal() const noexcept
        -> const internal::FilterOracle& = 0;
    virtual auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> = 0;
    virtual auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid> = 0;
    virtual auto IsSynchronized() const noexcept -> bool = 0;
    virtual auto IsWalletScanEnabled() const noexcept -> bool = 0;
    virtual auto JobReady(const PeerManager::Task type) const noexcept
        -> void = 0;
    virtual auto Mempool() const noexcept -> const internal::Mempool& = 0;
    virtual auto PeerTarget() const noexcept -> std::size_t = 0;
    virtual auto Reorg() const noexcept
        -> const network::zeromq::socket::Publish& = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const UnallocatedVector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto Submit(network::zeromq::Message&& work) const noexcept
        -> void = 0;
    virtual auto Track(network::zeromq::Message&& work) const noexcept
        -> std::future<void> = 0;
    virtual auto UpdateHeight(const block::Height height) const noexcept
        -> void = 0;
    virtual auto UpdateLocalHeight(
        const block::Position position) const noexcept -> void = 0;

    virtual auto FilterOracleInternal() noexcept -> internal::FilterOracle& = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;
    virtual auto StartWallet() noexcept -> void = 0;

    ~Network() override = default;
};

struct SyncDatabase {
    using Height = block::Height;
    using Items = UnallocatedVector<network::p2p::Block>;
    using Message = network::p2p::Data;

    virtual auto SyncTip() const noexcept -> block::Position = 0;

    virtual auto LoadSync(const Height height, Message& output) noexcept
        -> bool = 0;
    virtual auto ReorgSync(const Height height) noexcept -> bool = 0;
    virtual auto SetSyncTip(const block::Position& position) noexcept
        -> bool = 0;
    virtual auto StoreSync(
        const block::Position& tip,
        const Items& items) noexcept -> bool = 0;

    virtual ~SyncDatabase() = default;
};

struct Wallet : virtual public node::Wallet {
    enum class Task : OTZMQWorkType {
        index = OT_ZMQ_INTERNAL_SIGNAL + 0,
        scan = OT_ZMQ_INTERNAL_SIGNAL + 1,
        process = OT_ZMQ_INTERNAL_SIGNAL + 2,
        reorg = OT_ZMQ_INTERNAL_SIGNAL + 3,
    };

    virtual auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void = 0;
    virtual auto FeeEstimate() const noexcept -> std::optional<Amount> = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~Wallet() override = default;
};

struct SpendPolicy {
    bool unconfirmed_incoming_{false};
    bool unconfirmed_change_{true};
};

struct WalletDatabase {
    using NodeID = Identifier;
    using pNodeID = OTIdentifier;
    using SubchainIndex = Identifier;
    using pSubchainIndex = OTIdentifier;
    using Subchain = blockchain::crypto::Subchain;
    using SubchainID = std::pair<Subchain, pNodeID>;
    using ElementID = std::pair<Bip32Index, SubchainID>;
    using ElementMap = Map<Bip32Index, Vector<Vector<std::byte>>>;
    using Pattern = std::pair<ElementID, Vector<std::byte>>;
    using Patterns = Vector<Pattern>;
    using MatchingIndices = Vector<Bip32Index>;
    using MatchedTransaction = std::
        pair<MatchingIndices, std::shared_ptr<block::bitcoin::Transaction>>;
    using BlockMatches = Map<block::pTxid, MatchedTransaction>;
    using BatchedMatches = Map<block::Position, BlockMatches>;
    using UTXO = std::pair<
        blockchain::block::Outpoint,
        std::unique_ptr<block::bitcoin::Output>>;
    using KeyID = blockchain::crypto::Key;
    using TXOs =
        Map<blockchain::block::Outpoint,
            std::shared_ptr<const block::bitcoin::Output>>;

    virtual auto CompletedProposals() const noexcept
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance = 0;
    virtual auto GetBalance(const crypto::Key& key) const noexcept
        -> Balance = 0;
    virtual auto GetOutputs(
        node::TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const crypto::Key& key,
        TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag> = 0;
    virtual auto GetPatterns(
        const SubchainIndex& index,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Patterns = 0;
    virtual auto GetPosition() const noexcept -> block::Position = 0;
    virtual auto GetSubchainID(const NodeID& account, const Subchain subchain)
        const noexcept -> pSubchainIndex = 0;
    virtual auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> = 0;
    virtual auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid> = 0;
    virtual auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::pTxid> = 0;
    virtual auto GetUnspentOutputs(alloc::Resource* alloc = alloc::System())
        const noexcept -> Vector<UTXO> = 0;
    virtual auto GetUnspentOutputs(
        const NodeID& account,
        const Subchain subchain,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetWalletHeight() const noexcept -> block::Height = 0;
    virtual auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> = 0;
    virtual auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal> = 0;
    virtual auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<OTIdentifier> = 0;
    virtual auto PublishBalance() const noexcept -> void = 0;
    virtual auto SubchainLastIndexed(const SubchainIndex& index) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto SubchainLastScanned(const SubchainIndex& index) const noexcept
        -> block::Position = 0;
    virtual auto SubchainSetLastScanned(
        const SubchainIndex& index,
        const block::Position& position) const noexcept -> bool = 0;

    virtual auto AddConfirmedTransaction(
        const NodeID& account,
        const SubchainIndex& index,
        const block::Position& block,
        const std::size_t blockIndex,
        const Vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool = 0;
    virtual auto AddConfirmedTransactions(
        const NodeID& account,
        const SubchainIndex& index,
        const BatchedMatches& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool = 0;
    virtual auto AddMempoolTransaction(
        const NodeID& account,
        const Subchain subchain,
        const Vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction,
        TXOs& txoCreated) noexcept -> bool = 0;
    virtual auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) noexcept -> bool = 0;
    virtual auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool = 0;
    virtual auto AdvanceTo(const block::Position& pos) noexcept -> bool = 0;
    virtual auto CancelProposal(const Identifier& id) noexcept -> bool = 0;
    virtual auto FinalizeReorg(
        storage::lmdb::LMDB::Transaction& tx,
        const block::Position& pos) noexcept -> bool = 0;
    virtual auto ForgetProposals(
        const UnallocatedSet<OTIdentifier>& ids) noexcept -> bool = 0;
    virtual auto ReorgTo(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        const node::HeaderOracle& headers,
        const NodeID& account,
        const Subchain subchain,
        const SubchainIndex& index,
        const UnallocatedVector<block::Position>& reorg) noexcept -> bool = 0;
    virtual auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        SpendPolicy& policy) noexcept -> std::optional<UTXO> = 0;
    virtual auto StartReorg() noexcept -> storage::lmdb::LMDB::Transaction = 0;
    virtual auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) noexcept -> bool = 0;

    virtual ~WalletDatabase() = default;
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::node::internal
