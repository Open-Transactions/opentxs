// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "internal/core/Core.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/Bytes.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Data.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#endif  // OT_BLOCKCHAIN

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

class Core;
}  // namespace api
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Transaction;
struct Outpoint;
}  // namespace bitcoin

class Block;
class Header;
}  // namespace block

namespace client
{
class UpdateTransaction;
}  // namespace client

namespace internal
{
struct Database;
struct GCS;
}  // namespace internal

namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

#if OT_BLOCKCHAIN
namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static auto value(const api::Core&) -> blockchain::block::Height
    {
        return -1;
    }
};
template <>
struct make_blank<blockchain::block::Position> {
    static auto value(const api::Core& api) -> blockchain::block::Position
    {
        return {make_blank<blockchain::block::Height>::value(api),
                make_blank<blockchain::block::pHash>::value(api)};
    }
};
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
using BlockReader = ProtectedView<ReadView, std::shared_mutex, sLock>;
using BlockWriter = ProtectedView<WritableView, std::shared_mutex, eLock>;

enum class BlockStorage : std::uint8_t {
    None = 0,
    Cache = 1,
    All = 2,
};
}  // namespace opentxs::api::client::blockchain

namespace opentxs::blockchain::client
{
// parent hash, child hash
using ChainSegment = std::pair<block::pHash, block::pHash>;
using UpdatedHeader =
    std::map<block::pHash, std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = std::map<block::Height, block::pHash>;
using Hashes = std::set<block::pHash>;
using Segments = std::set<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = std::multimap<block::pHash, block::pHash>;
}  // namespace opentxs::blockchain::client
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::client::internal
{
#if OT_BLOCKCHAIN
struct BlockDatabase {
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> = 0;
    virtual auto BlockPolicy() const noexcept
        -> api::client::blockchain::BlockStorage = 0;
    virtual auto BlockStore(const block::Block& block) const noexcept
        -> bool = 0;

    virtual ~BlockDatabase() = default;
};

struct BlockOracle : virtual public opentxs::blockchain::client::BlockOracle {
    enum class Task : OTZMQWorkType {
        ProcessBlock = 0,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    virtual auto SubmitBlock(const zmq::Frame& in) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Run() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~BlockOracle() override = default;
};

struct FilterDatabase {
    using Hash = block::pHash;
    /// block hash, filter header, filter hash
    using Header = std::tuple<block::pHash, block::pHash, ReadView>;
    using Filter =
        std::pair<ReadView, std::unique_ptr<const blockchain::internal::GCS>>;

    virtual auto FilterHeaderTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto FilterTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool = 0;
    virtual auto HaveFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept -> bool = 0;
    virtual auto LoadFilter(const filter::Type type, const ReadView block) const
        noexcept -> std::unique_ptr<const blockchain::internal::GCS> = 0;
    virtual auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto SetFilterHeaderTip(
        const filter::Type type,
        const block::Position position) const noexcept -> bool = 0;
    virtual auto SetFilterTip(
        const filter::Type type,
        const block::Position position) const noexcept -> bool = 0;
    virtual auto StoreFilters(
        const filter::Type type,
        std::vector<Filter> filters) const noexcept -> bool = 0;
    virtual auto StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept -> bool = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle {
    virtual auto AddFilter(zmq::Message& work) const noexcept -> void = 0;
    virtual auto AddHeaders(zmq::Message& work) const noexcept -> void = 0;
    virtual auto CheckBlocks() const noexcept -> void = 0;
    virtual auto DefaultType() const noexcept -> filter::Type = 0;
    virtual auto LoadFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> std::unique_ptr<const blockchain::internal::GCS> = 0;

    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~FilterOracle() = default;
};

struct HeaderOracle : virtual public opentxs::blockchain::client::HeaderOracle {
    virtual auto Init() noexcept -> void = 0;
    using CheckpointsTypeData =
        std::tuple<block::Height, std::string, std::string>;
    using CheckpointsType = std::map<blockchain::Type, CheckpointsTypeData>;
    using CheckpointData =
        std::tuple<block::Height, block::pHash, block::pHash>;

    virtual auto GetDefaultCheckpoint() const noexcept -> CheckpointData = 0;
    virtual ~HeaderOracle() = default;
};

struct HeaderDatabase {
    virtual auto ApplyUpdate(const client::UpdateTransaction& update) const
        noexcept -> bool = 0;
    // Throws std::out_of_range if no block at that position
    virtual auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash = 0;
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
    virtual auto RecentHashes() const noexcept -> std::vector<block::pHash> = 0;
    virtual auto SiblingHashes() const noexcept -> Hashes = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    virtual ~HeaderDatabase() = default;
};

struct IO {
    using tcp = boost::asio::ip::tcp;

    operator boost::asio::io_context&() const noexcept { return context_; }

    auto Connect(
        const Space& id,
        const tcp::endpoint& endpoint,
        tcp::socket& socket) const noexcept -> void;
    auto Receive(
        const Space& id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        tcp::socket& socket) const noexcept -> void;

    auto AddNetwork() noexcept -> void;
    auto Shutdown() noexcept -> void;

    IO(const api::Core& api) noexcept;
    ~IO();

private:
    const api::Core& api_;
    mutable std::mutex lock_;
    OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    mutable int next_buffer_;
    mutable std::map<int, Space> buffers_;
    mutable boost::asio::io_context context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    boost::thread_group thread_pool_;

    auto clear_buffer(const int id) const noexcept -> void;
    auto get_buffer(const std::size_t bytes) const noexcept
        -> std::pair<int, WritableView>;

    auto callback(zmq::Message& in) noexcept -> void;

    IO() = delete;
    IO(const IO&) = delete;
    IO(IO&&) = delete;
    auto operator=(const IO&) -> IO& = delete;
    auto operator=(IO &&) -> IO& = delete;
};

struct Network : virtual public opentxs::blockchain::Network {
    enum class Task : OTZMQWorkType {
        SubmitBlockHeader = 0,
        SubmitFilterHeader = 1,
        SubmitFilter = 2,
        SubmitBlock = 3,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    virtual auto API() const noexcept -> const api::internal::Core& = 0;
    virtual auto Blockchain() const noexcept
        -> const api::client::internal::Blockchain& = 0;
    virtual auto BlockOracle() const noexcept
        -> const internal::BlockOracle& = 0;
    virtual auto Chain() const noexcept -> Type = 0;
    virtual auto DB() const noexcept -> blockchain::internal::Database& = 0;
    virtual auto FilterOracle() const noexcept
        -> const internal::FilterOracle& = 0;
    virtual auto HeaderOracle() const noexcept
        -> const internal::HeaderOracle& = 0;
    virtual auto IsSynchronized() const noexcept -> bool = 0;
    virtual auto Reorg() const noexcept
        -> const network::zeromq::socket::Publish& = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool = 0;
    virtual auto RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool = 0;
    virtual auto Submit(network::zeromq::Message& work) const noexcept
        -> void = 0;
    virtual auto UpdateHeight(const block::Height height) const noexcept
        -> void = 0;
    virtual auto UpdateLocalHeight(const block::Position position) const
        noexcept -> void = 0;
    virtual auto Work(const Task type) const noexcept -> OTZMQMessage = 0;

    virtual auto HeaderOracle() noexcept -> client::HeaderOracle& = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~Network() = default;
};

struct PeerDatabase {
    using Address = std::unique_ptr<p2p::internal::Address>;
    using Protocol = p2p::Protocol;
    using Service = p2p::Service;
    using Type = p2p::Network;

    virtual auto AddOrUpdate(Address address) const noexcept -> bool = 0;
    virtual auto Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address = 0;
    virtual auto Import(std::vector<Address> peers) const noexcept -> bool = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task : OTZMQWorkType {
        Getheaders = 0,
        Getcfheaders = 1,
        Getcfilters = 2,
        Heartbeat = 3,
        Getblock = 4,
        Body = 126,
        Header = 127,
        Connect = OT_ZMQ_CONNECT_SIGNAL,
        Disconnect = OT_ZMQ_DISCONNECT_SIGNAL,
        ReceiveMessage = OT_ZMQ_RECEIVE_SIGNAL,
        SendMessage = OT_ZMQ_SEND_SIGNAL,
        Register = OT_ZMQ_REGISTER_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Database() const noexcept -> const PeerDatabase& = 0;
    virtual auto Disconnect(const int id) const noexcept -> void = 0;
    virtual auto Endpoint(const Task type) const noexcept -> std::string = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool = 0;
    virtual auto RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool = 0;
    virtual auto RequestHeaders() const noexcept -> bool = 0;

    virtual auto init() noexcept -> void = 0;
    virtual auto Run() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~PeerManager() = default;
};

struct ThreadPool {
    using Future = std::shared_future<void>;

    enum class Work : OTZMQWorkType {
        Wallet = 0,
    };

    virtual auto Endpoint() const noexcept -> std::string = 0;
    virtual auto Reset(const Type chain) const noexcept -> void = 0;
    virtual auto Stop(const Type chain) const noexcept -> Future = 0;

    virtual ~ThreadPool() = default;
};

struct Wallet {
    enum class Task : OTZMQWorkType {
        index = 0,
        scan = 1,
        process = 2,
    };

    static auto ProcessTask(const zmq::Message& task) noexcept -> void;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Run() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~Wallet() = default;
};

struct WalletDatabase {
    using FilterType = filter::Type;
    using NodeID = Identifier;
    using pNodeID = OTIdentifier;
    using Subchain = api::client::blockchain::Subchain;
    using SubchainID = std::pair<Subchain, pNodeID>;
    using ElementID = std::pair<Bip32Index, SubchainID>;
    using ElementMap = std::map<Bip32Index, std::vector<Space>>;
    using Pattern = std::pair<ElementID, Space>;
    using Patterns = std::vector<Pattern>;
    using MatchingIndices = std::vector<Bip32Index>;
    using UTXO = std::pair<
        blockchain::block::bitcoin::Outpoint,
        proto::BlockchainTransactionOutput>;

    static const VersionNumber DefaultIndexVersion;

    virtual auto AddConfirmedTransaction(
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> Patterns = 0;
    virtual auto GetUnspentOutputs() const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> Patterns = 0;
    virtual auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> bool = 0;
    virtual auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool = 0;
    virtual auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber = 0;
    virtual auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> bool = 0;
    virtual auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position = 0;
    virtual auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position = 0;
    virtual auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool = 0;

    virtual ~WalletDatabase() = default;
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::client::internal

#if OT_BLOCKCHAIN
namespace opentxs::factory
{
auto BlockchainDatabase(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::client::internal::Network& network,
    const api::client::blockchain::database::implementation::Database& db,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::internal::Database>;
auto BlockchainFilterOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::FilterDatabase& database,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::FilterOracle>;
OPENTXS_EXPORT auto BlockchainNetworkBitcoin(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::Type type,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::Network>;
auto BlockchainPeerManager(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::PeerDatabase& database,
    const blockchain::client::internal::IO& io,
    const blockchain::Type type,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::PeerManager>;
OPENTXS_EXPORT auto BlockchainWallet(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::client::internal::Network& parent,
    const blockchain::Type chain,
    const std::string& shutdown)
    -> std::unique_ptr<blockchain::client::internal::Wallet>;
auto BlockOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::BlockOracle>;
auto HeaderOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::HeaderDatabase& database,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::client::internal::HeaderOracle>;
}  // namespace opentxs::factory
#endif  // OT_BLOCKCHAIN
