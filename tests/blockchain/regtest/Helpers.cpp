// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "1_Internal.hpp"  // IWYU pragma: keep
#include "integration/Helpers.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/Request.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "paymentcode/VectorsV3.hpp"

namespace ottest
{
using Dir = zmq::socket::Socket::Direction;

constexpr auto sync_server_sync_{"inproc://sync_server_endpoint/sync"};
constexpr auto sync_server_sync_public_{
    "inproc://sync_server_public_endpoint/sync"};
constexpr auto sync_server_update_{"inproc://sync_server_endpoint/update"};

struct BlockListener::Imp {
    const ot::api::Session& api_;
    mutable std::mutex lock_;
    std::promise<Position> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    Imp(const ot::api::Session& api)
        : api_(api)
        , lock_()
        , promise_()
        , target_(-1)
        , cb_(zmq::ListenCallback::Factory([&](zmq::Message&& msg) {
            const auto body = msg.Body();

            OT_ASSERT(0 < body.size());

            auto position = [&]() -> Position {
                switch (body.at(0).as<ot::WorkType>()) {
                    case ot::WorkType::BlockchainNewHeader: {
                        OT_ASSERT(3 < body.size());

                        return {body.at(3).as<Height>(), [&] {
                                    auto output = api_.Factory().Data();
                                    output->Assign(body.at(2).Bytes());

                                    return output;
                                }()};
                    }
                    case ot::WorkType::BlockchainReorg: {
                        OT_ASSERT(5 < body.size());

                        return {body.at(5).as<Height>(), [&] {
                                    auto output = api_.Factory().Data();
                                    output->Assign(body.at(4).Bytes());

                                    return output;
                                }()};
                    }
                    default: {

                        OT_FAIL;
                    }
                }
            }();
            const auto& [height, hash] = position;
            auto lock = ot::Lock{lock_};

            if (height == target_) {
                try {
                    promise_.set_value(std::move(position));
                } catch (...) {
                }
            }
        }))
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainReorg()));
    }
};

BlockListener::BlockListener(const ot::api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto BlockListener::GetFuture(const Height height) noexcept
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

BlockListener::~BlockListener() = default;

struct MinedBlocks::Imp {
    using Vector = std::pmr::vector<Future>;

    mutable std::mutex lock_{};
    Vector hashes_{};
};

MinedBlocks::MinedBlocks()
    : imp_(std::make_unique<Imp>())
{
}

auto MinedBlocks::allocate() noexcept -> Promise
{
    auto lock = ot::Lock{imp_->lock_};
    auto promise = Promise{};
    imp_->hashes_.emplace_back(promise.get_future());

    return promise;
}

auto MinedBlocks::get(const std::size_t index) const -> Future
{
    auto lock = ot::Lock{imp_->lock_};

    if (index >= imp_->hashes_.size()) {
        throw std::out_of_range("Invalid index");
    }

    return imp_->hashes_.at(index);
}

MinedBlocks::~MinedBlocks() = default;

struct PeerListener::Imp {
    PeerListener& parent_;
    const int client_count_;
    mutable std::mutex lock_;
    ot::OTZMQListenCallback miner_cb_;
    ot::OTZMQListenCallback client_1_cb_;
    ot::OTZMQListenCallback client_2_cb_;
    ot::OTZMQSubscribeSocket m_socket_;
    ot::OTZMQSubscribeSocket c1_socket_;
    ot::OTZMQSubscribeSocket c2_socket_;

    auto cb(zmq::Message&& msg, std::atomic_int& counter) noexcept -> void
    {
        const auto body = msg.Body();

        OT_ASSERT(2 < body.size());

        ++counter;

        auto lock = ot::Lock{lock_};

        if (client_count_ != parent_.miner_peers_) { return; }

        if ((0 < client_count_) && (0 == parent_.client_1_peers_)) { return; }

        if ((1 < client_count_) && (0 == parent_.client_2_peers_)) { return; }

        try {
            parent_.promise_.set_value();
        } catch (...) {
        }
    }

    Imp(PeerListener& parent,
        const bool waitForHandshake,
        const int clientCount,
        const ot::api::session::Client& miner,
        const ot::api::session::Client& client1,
        const ot::api::session::Client& client2)
        : parent_(parent)
        , client_count_(clientCount)
        , lock_()
        , miner_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&& msg) { cb(std::move(msg), parent_.miner_peers_); }))
        , client_1_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.client_1_peers_);
              }))
        , client_2_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.client_2_peers_);
              }))
        , m_socket_(miner.Network().ZeroMQ().SubscribeSocket(miner_cb_))
        , c1_socket_(client1.Network().ZeroMQ().SubscribeSocket(client_1_cb_))
        , c2_socket_(client2.Network().ZeroMQ().SubscribeSocket(client_2_cb_))
    {
        if (false == m_socket_->Start(
                         waitForHandshake
                             ? miner.Endpoints().BlockchainPeer()
                             : miner.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to miner socket");
        }

        if (false ==
            c1_socket_->Start(
                waitForHandshake
                    ? client1.Endpoints().BlockchainPeer()
                    : client1.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to client1 socket");
        }

        if (false ==
            c2_socket_->Start(
                waitForHandshake
                    ? client2.Endpoints().BlockchainPeer()
                    : client2.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to client2 socket");
        }
    }

    ~Imp()
    {
        c2_socket_->Close();
        c1_socket_->Close();
        m_socket_->Close();
    }
};

PeerListener::PeerListener(
    const bool waitForHandshake,
    const int clientCount,
    const ot::api::session::Client& miner,
    const ot::api::session::Client& client1,
    const ot::api::session::Client& client2)
    : promise_()
    , done_(promise_.get_future())
    , miner_peers_(0)
    , client_1_peers_(0)
    , client_2_peers_(0)
    , imp_(std::make_unique<
           Imp>(*this, waitForHandshake, clientCount, miner, client1, client2))
{
}

PeerListener::~PeerListener() = default;

Regtest_fixture_base::Regtest_fixture_base(
    const bool waitForHandshake,
    const int clientCount,
    const ot::Options& minerArgs,
    const ot::Options& clientArgs)
    : ot_(ot::Context())
    , client_args_(clientArgs)
    , client_count_(clientCount)
    , miner_(ot_.StartClientSession(
          ot::Options{minerArgs}
              .SetBlockchainStorageLevel(2)
              .SetBlockchainWalletEnabled(false),
          0))
    , client_1_(ot_.StartClientSession(client_args_, 1))
    , client_2_(ot_.StartClientSession(client_args_, 2))
    , address_(init_address(miner_))
    , connection_(init_peer(
          waitForHandshake,
          client_count_,
          miner_,
          client_1_,
          client_2_))
    , default_([&](Height height) -> Transaction {
        using OutputBuilder = ot::api::session::Factory::OutputBuilder;

        return miner_.Factory().BitcoinGenerationTransaction(
            test_chain_,
            height,
            [&] {
                auto output = std::pmr::vector<OutputBuilder>{};
                const auto text = std::string{"null"};
                const auto keys = std::pmr::set<ot::blockchain::crypto::Key>{};
                output.emplace_back(
                    5000000000,
                    miner_.Factory().BitcoinScriptNullData(test_chain_, {text}),
                    keys);

                return output;
            }(),
            coinbase_fun_);
    })
    , mined_blocks_(init_mined())
    , block_1_(init_block(0, client_1_))
    , block_2_(init_block(1, client_2_))
    , wallet_1_(init_wallet(0, client_1_))
    , wallet_2_(init_wallet(1, client_2_))
{
}

Regtest_fixture_base::Regtest_fixture_base(
    const bool waitForHandshake,
    const int clientCount,
    const ot::Options& clientArgs)
    : Regtest_fixture_base(
          waitForHandshake,
          clientCount,
          ot::Options{},
          clientArgs)
{
}

auto Regtest_fixture_base::compare_outpoints(
    const ot::blockchain::node::Wallet& wallet,
    const TXOState::Data& data) const noexcept -> bool
{
    auto output{true};

    for (const auto state : states_) {
        output &= compare_outpoints(state, data, wallet.GetOutputs(state));
    }

    return output;
}

auto Regtest_fixture_base::compare_outpoints(
    const ot::blockchain::node::Wallet& wallet,
    const ot::identifier::Nym& nym,
    const TXOState::Data& data) const noexcept -> bool
{
    auto output{true};

    for (const auto state : states_) {
        output &= compare_outpoints(state, data, wallet.GetOutputs(nym, state));
    }

    return output;
}

auto Regtest_fixture_base::compare_outpoints(
    const ot::blockchain::node::Wallet& wallet,
    const ot::identifier::Nym& nym,
    const ot::Identifier& subaccount,
    const TXOState::Data& data) const noexcept -> bool
{
    auto output{true};

    for (const auto state : states_) {
        output &= compare_outpoints(
            state, data, wallet.GetOutputs(nym, subaccount, state));
    }

    return output;
}

auto Regtest_fixture_base::compare_outpoints(
    const ot::blockchain::node::TxoState type,
    const TXOState::Data& expected,
    const std::pmr::vector<UTXO>& got) const noexcept -> bool
{
    auto output{true};
    static const auto emptySet =
        std::pmr::set<ot::blockchain::block::Outpoint>{};
    const auto& set = [&]() -> auto&
    {
        try {

            return expected.data_.at(type);
        } catch (...) {

            return emptySet;
        }
    }
    ();
    output &= set.size() == got.size();

    EXPECT_EQ(set.size(), got.size());

    for (const auto& [outpoint, pOutput] : got) {
        const auto have = (1 == set.count(outpoint));
        const auto match = TestUTXOs(expected_, got);

        output &= have;
        output &= match;

        EXPECT_TRUE(have);
        EXPECT_TRUE(match);
    }

    return output;
}

auto Regtest_fixture_base::Connect() noexcept -> bool
{
    return Connect(address_);
}

auto Regtest_fixture_base::Connect(const b::p2p::Address& address) noexcept
    -> bool
{
    const auto miner = [&]() -> std::function<bool()> {
        const auto& miner = miner_.Network().Blockchain().GetChain(test_chain_);
        const auto listen = miner.Listen(address);

        EXPECT_TRUE(listen);

        return [=] {
            EXPECT_EQ(connection_.miner_peers_, client_count_);

            return listen && (client_count_ == connection_.miner_peers_);
        };
    }();
    const auto client1 = [&]() -> std::function<bool()> {
        if (0 < client_count_) {
            const auto& client =
                client_1_.Network().Blockchain().GetChain(test_chain_);
            const auto added = client.AddPeer(address);

            EXPECT_TRUE(added);

            return [=] {
                EXPECT_EQ(connection_.client_1_peers_, 1);

                return added && (1 == connection_.client_1_peers_);
            };
        } else {

            return [] { return true; };
        }
    }();
    const auto client2 = [&]() -> std::function<bool()> {
        if (1 < client_count_) {
            const auto& client =
                client_2_.Network().Blockchain().GetChain(test_chain_);
            const auto added = client.AddPeer(address);

            EXPECT_TRUE(added);

            return [=] {
                EXPECT_EQ(connection_.client_2_peers_, 1);

                return added && (1 == connection_.client_2_peers_);
            };
        } else {

            return [] { return true; };
        }
    }();

    static constexpr auto limit = std::chrono::seconds{30};
    const auto status = connection_.done_.wait_for(limit);
    const auto future = (std::future_status::ready == status);

    OT_ASSERT(future);

    return future && miner() && client1() && client2();
}

auto Regtest_fixture_base::get_bytes(const Script& script) noexcept
    -> std::optional<ot::ReadView>
{
    switch (script.Type()) {
        case Pattern::PayToPubkey: {

            return script.Pubkey();
        }
        case Pattern::PayToPubkeyHash: {

            return script.PubkeyHash();
        }
        case Pattern::PayToMultisig: {

            return script.MultisigPubkey(0);
        }
        default: {

            return std::nullopt;
        }
    }
}

auto Regtest_fixture_base::init_address(const ot::api::Session& api) noexcept
    -> const b::p2p::Address&
{
    constexpr auto test_endpoint{"inproc://test_endpoint"};
    constexpr auto test_port = std::uint16_t{18444};

    if (false == bool(listen_address_)) {
        listen_address_ = std::make_unique<ot::OTBlockchainAddress>(
            api.Factory().BlockchainAddress(
                b::p2p::Protocol::bitcoin,
                b::p2p::Network::zmq,
                api.Factory().Data(
                    std::string{test_endpoint}, ot::StringStyle::Raw),
                test_port,
                test_chain_,
                {},
                {}));
    }

    OT_ASSERT(listen_address_);

    return *listen_address_;
}

auto Regtest_fixture_base::init_block(
    const int index,
    const ot::api::Session& api) noexcept -> BlockListener&
{
    auto& p = block_listener_[index];

    if (false == bool(p)) { p = std::make_unique<BlockListener>(api); }

    OT_ASSERT(p);

    return *p;
}

auto Regtest_fixture_base::init_mined() noexcept -> MinedBlocks&
{
    if (false == bool(mined_block_cache_)) {
        mined_block_cache_ = std::make_unique<MinedBlocks>();
    }

    OT_ASSERT(mined_block_cache_);

    return *mined_block_cache_;
}

auto Regtest_fixture_base::init_peer(
    const bool waitForHandshake,
    const int clientCount,
    const ot::api::session::Client& miner,
    const ot::api::session::Client& client1,
    const ot::api::session::Client& client2) noexcept -> const PeerListener&
{
    if (false == bool(peer_listener_)) {
        peer_listener_ = std::make_unique<PeerListener>(
            waitForHandshake, clientCount, miner, client1, client2);
    }

    OT_ASSERT(peer_listener_);

    return *peer_listener_;
}

auto Regtest_fixture_base::init_wallet(
    const int index,
    const ot::api::Session& api) noexcept -> WalletListener&
{
    auto& p = wallet_listener_[index];

    if (false == bool(p)) { p = std::make_unique<WalletListener>(api); }

    OT_ASSERT(p);

    return *p;
}

auto Regtest_fixture_base::MaturationInterval() noexcept
    -> ot::blockchain::block::Height
{
    static const auto interval = ot::blockchain::params::Data::Chains()
                                     .at(test_chain_)
                                     .maturation_interval_;

    return interval;
}

auto Regtest_fixture_base::Mine(
    const Height ancestor,
    const std::size_t count) noexcept -> bool
{
    return Mine(ancestor, count, default_);
}

auto Regtest_fixture_base::Mine(
    const Height ancestor,
    const std::size_t count,
    const Generator& gen,
    const std::pmr::vector<Transaction>& extra) noexcept -> bool
{
    const auto targetHeight = ancestor + static_cast<Height>(count);
    auto blocks = std::pmr::vector<BlockListener::Future>{};
    auto wallets = std::pmr::vector<WalletListener::Future>{};
    blocks.reserve(client_count_);
    wallets.reserve(client_count_);

    if (0 < client_count_) {
        blocks.emplace_back(block_1_.GetFuture(targetHeight));
        wallets.emplace_back(wallet_1_.GetFuture(targetHeight));
    }

    if (1 < client_count_) {
        blocks.emplace_back(block_2_.GetFuture(targetHeight));
        wallets.emplace_back(wallet_2_.GetFuture(targetHeight));
    }

    const auto& network = miner_.Network().Blockchain().GetChain(test_chain_);
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader =
        headerOracle.LoadHeader(headerOracle.BestHash(ancestor))->as_Bitcoin();

    OT_ASSERT(previousHeader);

    for (auto i = std::size_t{0u}; i < count; ++i) {
        auto promise = mined_blocks_.allocate();

        OT_ASSERT(gen);

        auto tx = gen(previousHeader->Height() + 1);

        OT_ASSERT(tx);

        auto block = miner_.Factory().BitcoinBlock(
            *previousHeader,
            tx,
            previousHeader->nBits(),
            extra,
            previousHeader->Version(),
            [start{ot::Clock::now()}] {
                return (ot::Clock::now() - start) > std::chrono::minutes(2);
            });

        OT_ASSERT(block);

        promise.set_value(block->Header().Hash());
        const auto added = network.AddBlock(block);

        OT_ASSERT(added);

        previousHeader = block->Header().as_Bitcoin();

        OT_ASSERT(previousHeader);
    }

    auto output = true;
    constexpr auto limit = std::chrono::minutes(5);
    using Status = std::future_status;

    for (auto& future : blocks) {
        OT_ASSERT(future.wait_for(limit) == Status::ready);

        const auto [height, hash] = future.get();

        EXPECT_EQ(hash, previousHeader->Hash());

        output &= (hash == previousHeader->Hash());
    }

    for (auto& future : wallets) {
        OT_ASSERT(future.wait_for(limit) == Status::ready);

        const auto height = future.get();

        EXPECT_EQ(height, targetHeight);

        output &= (height == targetHeight);
    }

    if (output) { height_ = targetHeight; }

    return output;
}

auto Regtest_fixture_base::Shutdown() noexcept -> void
{
    wallet_listener_.clear();
    block_listener_.clear();
    mined_block_cache_.reset();
    peer_listener_.reset();
    listen_address_.reset();
}

auto Regtest_fixture_base::Start() noexcept -> bool
{
    const auto miner = [&]() -> std::function<bool()> {
        const auto start = miner_.Network().Blockchain().Start(test_chain_);

        return [=] {
            EXPECT_TRUE(start);

            return start;
        };
    }();
    const auto client1 = [&]() -> std::function<bool()> {
        if (0 < client_count_) {
            const auto start =
                client_1_.Network().Blockchain().Start(test_chain_);

            return [=] {
                EXPECT_TRUE(start);

                return start;
            };
        } else {

            return [] { return true; };
        }
    }();
    const auto client2 = [&]() -> std::function<bool()> {
        if (1 < client_count_) {
            const auto start =
                client_2_.Network().Blockchain().Start(test_chain_);

            return [=] {
                EXPECT_TRUE(start);

                return start;
            };
        } else {

            return [] { return true; };
        }
    }();

    return miner() && client1() && client2();
}

auto Regtest_fixture_base::TestUTXOs(
    const Expected& expected,
    const std::pmr::vector<UTXO>& utxos) const noexcept -> bool
{
    auto out = true;

    for (const auto& utxo : utxos) {
        const auto& [outpoint, pOutput] = utxo;

        EXPECT_TRUE(pOutput);

        if (!pOutput) {
            out = false;

            continue;
        }

        const auto& output = *pOutput;

        try {
            const auto& [exKey, exAmount, exPattern] = expected.at(outpoint);
            out &= (output.Value() == exAmount);

            EXPECT_EQ(output.Value(), exAmount);

            const auto& script = output.Script();
            using Position = ot::blockchain::block::bitcoin::Script::Position;
            out &= (script.Role() == Position::Output);

            EXPECT_EQ(script.Role(), Position::Output);

            const auto data = get_bytes(script);

            EXPECT_TRUE(data.has_value());

            if (false == data.has_value()) {
                out = false;

                continue;
            }

            out &= (data.value() == exKey->Bytes());
            out &= (script.Type() == exPattern);

            EXPECT_EQ(data.value(), exKey->Bytes());
            EXPECT_EQ(script.Type(), exPattern);
        } catch (...) {
            EXPECT_EQ(outpoint.str(), "this will never be true");

            out = false;
        }
    }

    return out;
}

auto Regtest_fixture_base::TestWallet(
    const ot::api::session::Client& api,
    const TXOState& state) const noexcept -> bool
{
    auto output{true};
    const auto& network = api.Network().Blockchain().GetChain(test_chain_);
    const auto& wallet = network.Wallet();
    using Balance = ot::blockchain::Balance;
    static const auto blankNym = api.Factory().NymID();
    static const auto blankAccount = api.Factory().Identifier();
    static const auto noBalance = Balance{0, 0};
    static const auto blankData = TXOState::Data{};
    const auto test2 = [&](const auto& eBalance,
                           const auto wBalance,
                           const auto nBalance,
                           const auto outpoints) {
        auto output{true};
        output &= (wBalance == eBalance);
        output &= (nBalance == eBalance);
        output &= outpoints;

        EXPECT_EQ(wBalance.first, eBalance.first);
        EXPECT_EQ(wBalance.second, eBalance.second);
        EXPECT_EQ(nBalance.first, eBalance.first);
        EXPECT_EQ(nBalance.second, eBalance.second);
        EXPECT_TRUE(outpoints);

        return output;
    };
    const auto test =
        [&](const auto& eBalance, const auto wBalance, const auto outpoints) {
            return test2(eBalance, wBalance, wBalance, outpoints);
        };
    output &= test2(
        state.wallet_.balance_,
        wallet.GetBalance(),
        network.GetBalance(),
        compare_outpoints(wallet, state.wallet_));
    output &= test2(
        noBalance,
        wallet.GetBalance(blankNym),
        network.GetBalance(blankNym),
        compare_outpoints(wallet, blankNym, blankData));
    output &= test(
        noBalance,
        wallet.GetBalance(blankNym, blankAccount),
        compare_outpoints(wallet, blankNym, blankAccount, blankData));

    for (const auto& [nymID, nymData] : state.nyms_) {
        output &= test2(
            nymData.nym_.balance_,
            wallet.GetBalance(nymID),
            network.GetBalance(nymID),
            compare_outpoints(wallet, nymID, nymData.nym_));
        output &= test(
            noBalance,
            wallet.GetBalance(nymID, blankAccount),
            compare_outpoints(wallet, nymID, blankAccount, blankData));

        for (const auto& [accountID, accountData] : nymData.accounts_) {
            output &= test(
                accountData.balance_,
                wallet.GetBalance(nymID, accountID),
                compare_outpoints(wallet, nymID, accountID, nymData.nym_));
            output &= test(
                noBalance,
                wallet.GetBalance(blankNym, accountID),
                compare_outpoints(wallet, blankNym, accountID, blankData));
        }
    }

    return output;
}

Regtest_fixture_hd::Regtest_fixture_hd()
    : Regtest_fixture_normal(1)
    , expected_notary_(client_1_.UI().BlockchainNotaryID(test_chain_))
    , expected_unit_(client_1_.UI().BlockchainUnitID(test_chain_))
    , expected_display_unit_(u8"UNITTEST")
    , expected_account_name_(u8"This device")
    , expected_notary_name_(u8"Unit Test Simulation")
    , memo_outgoing_("memo for outgoing transaction")
    , expected_account_type_(ot::AccountType::Blockchain)
    , expected_unit_type_(ot::core::UnitType::Regtest)
    , hd_generator_([&](Height height) -> Transaction {
        using OutputBuilder = ot::api::session::Factory::OutputBuilder;
        using Index = ot::Bip32Index;
        static constexpr auto count = 100u;
        static const auto baseAmount = ot::blockchain::Amount{100000000};
        auto meta = std::pmr::vector<OutpointMetadata>{};
        meta.reserve(count);
        const auto& account = SendHD();
        auto output = miner_.Factory().BitcoinGenerationTransaction(
            test_chain_,
            height,
            [&] {
                auto output = std::pmr::vector<OutputBuilder>{};
                const auto reason =
                    client_1_.Factory().PasswordPrompt(__func__);
                const auto keys = std::pmr::set<ot::blockchain::crypto::Key>{};

                for (auto i = Index{0}; i < Index{count}; ++i) {
                    const auto index = account.Reserve(
                        Subchain::External,
                        client_1_.Factory().PasswordPrompt(""));
                    const auto& element = account.BalanceElement(
                        Subchain::External, index.value_or(0));
                    const auto key = element.Key();

                    OT_ASSERT(key);

                    switch (i) {
                        case 0: {
                            const auto& [bytes, value, pattern] =
                                meta.emplace_back(
                                    client_1_.Factory().Data(
                                        element.Key()->PublicKey()),
                                    baseAmount + i,
                                    Pattern::PayToPubkey);
                            output.emplace_back(
                                value,
                                miner_.Factory().BitcoinScriptP2PK(
                                    test_chain_, *key),
                                keys);
                        } break;
                        default: {
                            const auto& [bytes, value, pattern] =
                                meta.emplace_back(
                                    element.PubkeyHash(),
                                    baseAmount + i,
                                    Pattern::PayToPubkeyHash);
                            output.emplace_back(
                                value,
                                miner_.Factory().BitcoinScriptP2PKH(
                                    test_chain_, *key),
                                keys);
                        }
                    }
                }

                return output;
            }(),
            coinbase_fun_);

        OT_ASSERT(output);

        const auto& txid = transactions_.emplace_back(output->ID()).get();

        for (auto i = Index{0}; i < Index{count}; ++i) {
            auto& [bytes, amount, pattern] = meta.at(i);
            expected_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(txid.Bytes(), i),
                std::forward_as_tuple(
                    std::move(bytes), std::move(amount), std::move(pattern)));
            txos_.AddGenerated(*output, i, account, height);
        }

        return output;
    })
    , listener_([&]() -> ScanListener& {
        if (!listener_p_) {
            listener_p_ = std::make_unique<ScanListener>(client_1_);
        }

        OT_ASSERT(listener_p_);

        return *listener_p_;
    }())
{
    if (false == init_) {
        auto cb = [](User& user) {
            const auto& api = *user.api_;
            const auto& nymID = user.nym_id_.get();
            const auto reason = api.Factory().PasswordPrompt(__func__);
            api.Crypto().Blockchain().NewHDSubaccount(
                nymID,
                ot::blockchain::crypto::HDProtocol::BIP_44,
                test_chain_,
                reason);
        };
        auto& alice = const_cast<User&>(alice_);
        alice.init_custom(client_1_, cb);

        OT_ASSERT(alice_.payment_code_ == GetVectors3().alice_.payment_code_);

        init_ = true;
    }
}

auto Regtest_fixture_hd::CheckTXODB() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_.Extract(out);

        return out;
    }();

    return TestWallet(client_1_, state);
}

auto Regtest_fixture_hd::SendHD() const noexcept -> const bca::HD&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alice_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_fixture_hd::Shutdown() noexcept -> void
{
    listener_p_.reset();
    transactions_.clear();
    Regtest_fixture_normal::Shutdown();
}

Regtest_fixture_normal::Regtest_fixture_normal(const int clientCount)
    : Regtest_fixture_base(
          true,
          clientCount,
          ot::Options{}.SetBlockchainStorageLevel(1))
{
}

Regtest_fixture_single::Regtest_fixture_single()
    : Regtest_fixture_normal(1)
{
}

Regtest_fixture_sync::Regtest_fixture_sync()
    : Regtest_fixture_base(
          true,
          2,
          ot::Options{}.SetBlockchainStorageLevel(2).SetBlockchainSyncEnabled(
              true))
    , sync_sub_([&]() -> SyncSubscriber& {
        if (!sync_subscriber_) {
            sync_subscriber_ =
                std::make_unique<SyncSubscriber>(client_2_, mined_blocks_);
        }

        return *sync_subscriber_;
    }())
    , sync_req_([&]() -> SyncRequestor& {
        if (!sync_requestor_) {
            sync_requestor_ =
                std::make_unique<SyncRequestor>(client_2_, mined_blocks_);
        }

        return *sync_requestor_;
    }())
{
    if (false == init_) {
        auto& alex = const_cast<User&>(alex_);
        alex.init(client_2_);

        OT_ASSERT(alex.payment_code_ == GetVectors3().alice_.payment_code_);

        init_ = true;
    }
}

auto Regtest_fixture_sync::Connect() noexcept -> bool
{
    auto output = Regtest_fixture_base::Connect();
    output &= client_1_.Network().Blockchain().StartSyncServer(
        sync_server_sync_,
        sync_server_sync_public_,
        sync_server_update_,
        sync_server_update_public_);

    return output;
}

auto Regtest_fixture_sync::Shutdown() noexcept -> void
{
    sync_requestor_.reset();
    sync_subscriber_.reset();
    Regtest_fixture_base::Shutdown();
}

Regtest_fixture_tcp::Regtest_fixture_tcp()
    : Regtest_fixture_base(
          false,
          1,
          ot::Options{},
          ot::Options{}.SetBlockchainStorageLevel(1))
    , tcp_listen_address_(miner_.Factory().BlockchainAddress(
          b::p2p::Protocol::bitcoin,
          b::p2p::Network::ipv4,
          miner_.Factory().Data("0x7f000001", ot::StringStyle::Hex),
          18444,
          test_chain_,
          {},
          {}))
{
}

auto Regtest_fixture_tcp::Connect() noexcept -> bool
{
    return Connect(tcp_listen_address_);
}

Regtest_payment_code::Regtest_payment_code()
    : Regtest_fixture_normal(2)
    , api_server_1_(ot::Context().StartNotarySession(0))
    , expected_notary_(client_1_.UI().BlockchainNotaryID(test_chain_))
    , expected_unit_(client_1_.UI().BlockchainUnitID(test_chain_))
    , expected_display_unit_(u8"UNITTEST")
    , expected_account_name_(u8"This device")
    , expected_notary_name_(u8"Unit Test Simulation")
    , memo_outgoing_("memo for outgoing transaction")
    , expected_account_type_(ot::AccountType::Blockchain)
    , expected_unit_type_(ot::core::UnitType::Regtest)
    , mine_to_alice_([&](Height height) -> Transaction {
        using OutputBuilder = ot::api::session::Factory::OutputBuilder;
        static const auto baseAmmount = ot::blockchain::Amount{10000000000};
        auto meta = std::pmr::vector<OutpointMetadata>{};
        const auto& account = SendHD();
        auto output = miner_.Factory().BitcoinGenerationTransaction(
            test_chain_,
            height,
            [&] {
                auto output = std::pmr::vector<OutputBuilder>{};
                const auto reason =
                    client_1_.Factory().PasswordPrompt(__func__);
                const auto keys = std::pmr::set<bca::Key>{};
                const auto index = account.Reserve(Subchain::External, reason);

                EXPECT_TRUE(index.has_value());

                const auto& element = account.BalanceElement(
                    Subchain::External, index.value_or(0));
                const auto key = element.Key();

                OT_ASSERT(key);

                const auto& [bytes, value, pattern] = meta.emplace_back(
                    client_1_.Factory().Data(element.Key()->PublicKey()),
                    baseAmmount,
                    Pattern::PayToPubkey);
                output.emplace_back(
                    value,
                    miner_.Factory().BitcoinScriptP2PK(test_chain_, *key),
                    keys);

                return output;
            }(),
            coinbase_fun_);

        OT_ASSERT(output);

        const auto& txid = transactions_.emplace_back(output->ID()).get();
        auto& [bytes, amount, pattern] = meta.at(0);
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 0),
            std::forward_as_tuple(
                std::move(bytes), std::move(amount), std::move(pattern)));
        txos_alice_.AddGenerated(*output, 0, account, height);

        return output;
    })
    , listener_alice_([&]() -> ScanListener& {
        if (!listener_alice_p_) {
            listener_alice_p_ = std::make_unique<ScanListener>(client_1_);
        }

        OT_ASSERT(listener_alice_p_);

        return *listener_alice_p_;
    }())
    , listener_bob_([&]() -> ScanListener& {
        if (!listener_bob_p_) {
            listener_bob_p_ = std::make_unique<ScanListener>(client_2_);
        }

        OT_ASSERT(listener_bob_p_);

        return *listener_bob_p_;
    }())
{
    if (false == init_) {
        server_1_.init(api_server_1_);
        set_introduction_server(miner_, server_1_);
        auto cb = [](User& user) {
            const auto& api = *user.api_;
            const auto& nymID = user.nym_id_.get();
            const auto reason = api.Factory().PasswordPrompt(__func__);
            api.Crypto().Blockchain().NewHDSubaccount(
                nymID,
                ot::blockchain::crypto::HDProtocol::BIP_44,
                test_chain_,
                reason);
        };
        auto& alice = const_cast<User&>(alice_);
        auto& bob = const_cast<User&>(bob_);
        alice.init_custom(client_1_, server_1_, cb);
        bob.init_custom(client_2_, server_1_, cb);

        OT_ASSERT(alice_.payment_code_ == GetVectors3().alice_.payment_code_);
        OT_ASSERT(bob_.payment_code_ == GetVectors3().bob_.payment_code_);

        init_ = true;
    }
}

auto Regtest_payment_code::CheckContactID(
    const User& local,
    const User& remote,
    const std::string& paymentcode) const noexcept -> bool
{
    const auto& api = *local.api_;
    const auto n2c = api.Contacts().ContactID(remote.nym_id_);
    const auto p2c = api.Contacts().PaymentCodeToContact(
        api.Factory().PaymentCode(paymentcode), test_chain_);

    EXPECT_EQ(n2c, p2c);
    EXPECT_EQ(p2c->str(), local.Contact(remote.name_).str());

    auto output{true};
    output &= (n2c == p2c);
    output &= (p2c->str() == local.Contact(remote.name_).str());

    return output;
}

auto Regtest_payment_code::CheckTXODBAlice() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_alice_.Extract(out);

        return out;
    }();

    return TestWallet(client_1_, state);
}

auto Regtest_payment_code::CheckTXODBBob() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_bob_.Extract(out);

        return out;
    }();

    return TestWallet(client_2_, state);
}

auto Regtest_payment_code::ReceiveHD() const noexcept -> const bca::HD&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(bob_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_payment_code::ReceivePC() const noexcept -> const bca::PaymentCode&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(bob_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_payment_code::SendHD() const noexcept -> const bca::HD&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alice_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_payment_code::SendPC() const noexcept -> const bca::PaymentCode&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alice_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_payment_code::Shutdown() noexcept -> void
{
    listener_bob_p_.reset();
    listener_alice_p_.reset();
    transactions_.clear();
    Regtest_fixture_normal::Shutdown();
}

struct ScanListener::Imp {
    using Chain = ot::blockchain::Type;
    using Position = ot::blockchain::block::Position;
    using Promise = std::promise<void>;

    struct Data {
        Position pos_;
        Height target_;
        Promise promise_;

        auto reset(Height target) noexcept -> Future
        {
            target_ = target;
            promise_ = {};

            return promise_.get_future();
        }
        auto test() noexcept -> void
        {
            if (pos_.first == target_) {
                try {
                    promise_.set_value();
                } catch (...) {
                }
            }
        }

        Data(Height height, ot::OTData&& hash) noexcept
            : pos_(height, std::move(hash))
            , target_()
            , promise_()
        {
        }
        Data(Data&&) = default;

    private:
        Data() = delete;
        Data(const Data&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using SubchainMap = std::pmr::map<Subchain, Data>;
    using AccountMap = std::pmr::map<ot::OTIdentifier, SubchainMap>;
    using ChainMap = std::pmr::map<Chain, AccountMap>;
    using Map = std::pmr::map<ot::OTNymID, ChainMap>;

    const ot::api::Session& api_;
    const ot::OTZMQListenCallback cb_;
    const ot::OTZMQSubscribeSocket socket_;
    mutable std::mutex lock_;
    Map map_;

    auto cb(ot::network::zeromq::Message&& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() == 8u);

        const auto chain = body.at(1).as<Chain>();
        auto nymID = [&] {
            auto out = api_.Factory().NymID();
            out->Assign(body.at(2).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        auto accountID = [&] {
            auto out = api_.Factory().Identifier();
            out->Assign(body.at(4).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        const auto sub = body.at(5).as<Subchain>();
        const auto height = body.at(6).as<Height>();
        auto hash = [&] {
            auto out = api_.Factory().Data();
            out->Assign(body.at(7).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        auto lock = ot::Lock{lock_};
        auto& map = map_[std::move(nymID)][chain][std::move(accountID)];
        auto it = [&] {
            if (auto i = map.find(sub); i != map.end()) {
                if (height > i->second.pos_.first) {
                    i->second.pos_ = Position{height, std::move(hash)};
                }

                return i;
            } else {
                return map.try_emplace(sub, height, std::move(hash)).first;
            }
        }();

        it->second.test();
    }

    Imp(const ot::api::Session& api) noexcept
        : api_(api)
        , cb_(Callback::Factory([&](auto&& msg) { cb(std::move(msg)); }))
        , socket_([&] {
            auto out = api_.Network().ZeroMQ().SubscribeSocket(cb_);
            const auto rc =
                out->Start(api_.Endpoints().BlockchainScanProgress());

            OT_ASSERT(rc);

            return out;
        }())
        , lock_()
        , map_()
    {
    }
};

ScanListener::ScanListener(const ot::api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto ScanListener::get_future(
    const Subaccount& account,
    Subchain subchain,
    Height target) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    const auto& nym = account.Parent().NymID();
    const auto chain = account.Parent().Parent().Chain();
    const auto& id = account.ID();
    auto& map = imp_->map_[nym][chain][id];
    auto it = [&] {
        if (auto i = map.find(subchain); i != map.end()) {

            return i;
        } else {
            return map.try_emplace(subchain, -1, imp_->api_.Factory().Data())
                .first;
        }
    }();

    return it->second.reset(target);
}

auto ScanListener::wait(const Future& future) const noexcept -> bool
{
    constexpr auto limit = std::chrono::minutes{5};
    using Status = std::future_status;

    if (Status::ready == future.wait_for(limit)) {

        return true;
    } else {

        return false;
    }
}

ScanListener::~ScanListener() = default;

struct SyncRequestor::Imp {
    using Buffer = std::pmr::deque<ot::network::zeromq::Message>;

    SyncRequestor& parent_;
    const ot::api::session::Client& api_;
    const MinedBlocks& cache_;
    mutable std::mutex lock_;
    std::atomic_int updated_;
    Buffer buffer_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQDealerSocket socket_;

    auto cb(ot::network::zeromq::Message&& in) noexcept -> void
    {
        auto lock = ot::Lock{lock_};
        buffer_.emplace_back(std::move(in));
        ++updated_;
    }

    Imp(SyncRequestor& parent,
        const ot::api::session::Client& api,
        const MinedBlocks& cache) noexcept
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , lock_()
        , updated_(0)
        , buffer_()
        , cb_(zmq::ListenCallback::Factory(
              [&](auto&& msg) { cb(std::move(msg)); }))
        , socket_(api.Network().ZeroMQ().DealerSocket(cb_, Dir::Connect))
    {
        socket_->Start(sync_server_sync_);
    }
};

SyncRequestor::SyncRequestor(
    const ot::api::session::Client& api,
    const MinedBlocks& cache) noexcept
    : checked_(-1)
    , expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncRequestor::check(const State& state, const Position& pos)
    const noexcept -> bool
{
    auto output{true};
    output &= (state.Chain() == test_chain_);
    output &= (state.Position() == pos);

    EXPECT_EQ(state.Chain(), test_chain_);
    EXPECT_EQ(state.Position(), pos);

    return output;
}

auto SyncRequestor::check(const State& state, const std::size_t index) const
    -> bool
{
    auto pos = [&] {
        const auto& chain =
            imp_->api_.Network().Blockchain().GetChain(test_chain_);
        auto header =
            chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

        OT_ASSERT(header);

        return header->Position();
    }();

    return check(state, pos);
}

auto SyncRequestor::check(const otsync::Block& block, const std::size_t index)
    const noexcept -> bool
{
    constexpr auto filterType{ot::blockchain::filter::Type::ES};
    const auto& chain = imp_->api_.Network().Blockchain().GetChain(test_chain_);
    const auto header =
        chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

    OT_ASSERT(header);

    auto headerBytes = ot::Space{};

    EXPECT_TRUE(header->Serialize(ot::writer(headerBytes)));

    const auto& pos = header->Position();
    auto output{true};
    output &= (block.Chain() == test_chain_);
    output &= (block.Height() == pos.first);
    output &= (block.Header() == ot::reader(headerBytes));
    output &= (block.FilterType() == filterType);
    // TODO verify filter

    EXPECT_EQ(block.Chain(), test_chain_);
    EXPECT_EQ(block.Height(), pos.first);
    EXPECT_EQ(block.Header(), ot::reader(headerBytes));
    EXPECT_EQ(block.FilterType(), filterType);

    return output;
}

auto SyncRequestor::get(const std::size_t index) const -> const zmq::Message&
{
    auto lock = ot::Lock{imp_->lock_};

    return imp_->buffer_.at(index);
}

auto SyncRequestor::request(const Position& pos) const noexcept -> bool
{
    return request(opentxs::factory::BlockchainSyncRequest([&] {
        auto out = otsync::StateData{};
        out.emplace_back(test_chain_, pos);

        return out;
    }()));
}

auto SyncRequestor::request(const otsync::Base& command) const noexcept -> bool
{
    try {
        return imp_->socket_->Send([&] {
            auto out = opentxs::network::zeromq::Message{};

            if (false == command.Serialize(out)) {
                throw std::runtime_error{"serialization error"};
            }

            return out;
        }());
    } catch (...) {
        EXPECT_TRUE(false);

        return false;
    }
}

auto SyncRequestor::wait(const bool hard) noexcept -> bool
{
    const auto limit =
        hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
    auto start = ot::Clock::now();

    while ((imp_->updated_ < expected_) &&
           ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    if (false == hard) { imp_->updated_.store(expected_.load()); }

    return imp_->updated_ >= expected_;
}

SyncRequestor::~SyncRequestor() = default;

struct SyncSubscriber::Imp {
    SyncSubscriber& parent_;
    const ot::api::session::Client& api_;
    const MinedBlocks& cache_;
    std::atomic_int updated_;
    std::atomic_int errors_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto check_update(ot::network::zeromq::Message&& in) noexcept -> void
    {
        namespace bcsync = ot::network::p2p;
        const auto base = api_.Factory().BlockchainSyncMessage(in);

        try {
            const auto& data = base->asData();
            const auto& state = data.State();
            const auto& blocks = data.Blocks();
            const auto index = updated_++;
            const auto future = cache_.get(index);
            const auto hash = future.get();

            if (bcsync::MessageType::new_block_header != base->Type()) {
                throw std::runtime_error{"invalid message"};
            }

            if (state.Chain() != test_chain_) {
                throw std::runtime_error{"wrong chain"};
            }

            if (0 == data.PreviousCfheader().size()) {
                throw std::runtime_error{"invalid previous cfheader"};
            }

            if (0 == blocks.size()) {
                throw std::runtime_error{"no block data"};
            }

            if (1 != blocks.size()) {
                throw std::runtime_error{"wrong number of blocks"};
            }

            if (state.Position().second != hash) {
                std::runtime_error("wrong hash");
            }
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            ++errors_;
        }
    }
    auto wait_for_counter(const bool hard = true) noexcept -> bool
    {
        const auto limit =
            hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
        auto start = ot::Clock::now();
        const auto& expected = parent_.expected_;

        while ((updated_ < expected) && ((ot::Clock::now() - start) < limit)) {
            ot::Sleep(std::chrono::milliseconds(100));
        }

        if (false == hard) { updated_.store(expected.load()); }

        return updated_ >= expected;
    }

    Imp(SyncSubscriber& parent,
        const ot::api::session::Client& api,
        const MinedBlocks& cache)
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , updated_(0)
        , errors_(0)
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [&](auto&& in) { check_update(std::move(in)); }))
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        if (false == socket_->Start(sync_server_update_)) {
            throw std::runtime_error("Failed to subscribe to updates");
        }
    }
};

SyncSubscriber::SyncSubscriber(
    const ot::api::session::Client& api,
    const MinedBlocks& cache)
    : expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncSubscriber::wait(const bool hard) noexcept -> bool
{
    if (imp_->wait_for_counter(hard)) {
        auto output = (0 == imp_->errors_);
        imp_->errors_ = 0;

        return output;
    }

    return false;
}

SyncSubscriber::~SyncSubscriber() = default;

TXOState::TXOState() noexcept
    : wallet_()
    , nyms_()
{
}

TXOState::NymData::NymData() noexcept
    : nym_()
    , accounts_()
{
}

TXOState::Data::Data() noexcept
    : balance_()
    , data_()
{
}

struct TXOs::Imp {
    auto AddConfirmed(
        const ot::blockchain::block::bitcoin::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
    {
        return add_to_map(tx, index, owner, confirmed_incoming_);
    }
    auto AddGenerated(
        const ot::blockchain::block::bitcoin::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        const ot::blockchain::block::Height position) noexcept -> bool
    {
        try {
            const auto& output = tx.Outputs().at(index);
            const auto [it, added] = immature_[position].emplace(
                std::piecewise_construct,
                std::forward_as_tuple(owner.ID()),
                std::forward_as_tuple(tx.ID(), index, output.Value()));

            return added;
        } catch (...) {

            return false;
        }
    }
    auto AddUnconfirmed(
        const ot::blockchain::block::bitcoin::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
    {
        return add_to_map(tx, index, owner, unconfirmed_incoming_);
    }
    auto Confirm(const ot::blockchain::block::Txid& txid) noexcept -> bool
    {
        auto confirmed =
            move_txos(txid, unconfirmed_incoming_, confirmed_incoming_);
        confirmed += move_txos(txid, unconfirmed_spent_, confirmed_spent_);

        return 0 < confirmed;
    }
    auto Mature(const ot::blockchain::block::Height pos) noexcept -> bool
    {
        auto output{true};

        for (auto i{immature_.begin()}; i != immature_.end();) {
            auto& [height, outputs] = *i;
            const auto mature =
                (pos - height) >= Regtest_fixture_base::MaturationInterval();

            if (mature) {
                for (auto& [subaccount, txo] : outputs) {
                    const auto [it, added] =
                        confirmed_incoming_[subaccount].emplace(txo);
                    output &= added;
                }

                i = immature_.erase(i);
            } else {
                break;
            }
        }

        return output;
    }
    auto Orphan(const ot::blockchain::block::Txid& txid) noexcept -> bool
    {
        return true;  // TODO
    }
    auto OrphanGeneration(const ot::blockchain::block::Txid& txid) noexcept
        -> bool
    {
        return true;  // TODO
    }
    auto SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool
    {
        return spend_txo(txo, unconfirmed_spent_);
    }
    auto SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool
    {
        return spend_txo(txo, confirmed_spent_);
    }

    auto Extract(TXOState& output) const noexcept -> void
    {
        using State = ot::blockchain::node::TxoState;
        static constexpr auto all{State::All};
        auto& nym = output.nyms_[user_.nym_id_];
        auto& [wConfirmed, wUnconfirmed] = output.wallet_.balance_;
        auto& [nConfirmed, nUnconfirmed] = nym.nym_.balance_;
        auto& wMap = output.wallet_.data_;
        auto& nMap = nym.nym_.data_;

        for (const auto& [account, txos] : confirmed_incoming_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += value;
                nConfirmed += value;
                aConfirmed += value;
                wUnconfirmed += value;
                nUnconfirmed += value;
                aUnconfirmed += value;
                static constexpr auto state{State::ConfirmedNew};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : unconfirmed_incoming_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += value;
                nUnconfirmed += value;
                aUnconfirmed += value;
                static constexpr auto state{State::UnconfirmedNew};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : confirmed_spent_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::ConfirmedSpend};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : unconfirmed_spent_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += value;
                nConfirmed += value;
                aConfirmed += value;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::UnconfirmedSpend};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [height, data] : immature_) {
            for (const auto& [account, txo] : data) {
                const auto& outpoint = txo.outpoint_;
                auto& aData = nym.accounts_[account];
                auto& [aConfirmed, aUnconfirmed] = aData.balance_;
                auto& aMap = aData.data_;
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::Immature};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }
    }

    Imp(const User& owner) noexcept
        : user_(owner)
        , unconfirmed_incoming_()
        , confirmed_incoming_()
        , unconfirmed_spent_()
        , confirmed_spent_()
        , immature_()
    {
    }

private:
    struct TXO {
        ot::blockchain::block::Outpoint outpoint_;
        ot::blockchain::Amount value_;

        auto operator<(const TXO& rhs) const noexcept -> bool
        {
            if (outpoint_ < rhs.outpoint_) { return true; }

            if (rhs.outpoint_ < outpoint_) { return false; }

            return value_ < rhs.value_;
        }

        TXO(const ot::blockchain::block::Txid& txid,
            const std::size_t index,
            ot::blockchain::Amount value)
        noexcept
            : outpoint_(txid.Bytes(), static_cast<std::uint32_t>(index))
            , value_(value)
        {
        }
        TXO(const TXO& rhs)
        noexcept
            : outpoint_(rhs.outpoint_)
            , value_(rhs.value_)
        {
        }

    private:
        TXO() = delete;
        TXO(TXO&&) = delete;
        auto operator=(const TXO&) -> TXO& = delete;
        auto operator=(TXO&&) -> TXO& = delete;
    };

    using TXOSet = std::pmr::set<TXO>;
    using Map = std::pmr::map<ot::OTIdentifier, TXOSet>;
    using Immature = std::pmr::map<
        ot::blockchain::block::Height,
        std::pmr::set<std::pair<ot::OTIdentifier, TXO>>>;

    const User& user_;
    Map unconfirmed_incoming_;
    Map confirmed_incoming_;
    Map unconfirmed_spent_;
    Map confirmed_spent_;
    Immature immature_;

    auto add_to_map(
        const ot::blockchain::block::bitcoin::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        Map& map) noexcept -> bool
    {
        try {
            const auto& output = tx.Outputs().at(index);
            const auto [it, added] =
                map[owner.ID()].emplace(tx.ID(), index, output.Value());

            return added;
        } catch (...) {

            return false;
        }
    }
    auto move_txo(
        const ot::blockchain::block::Outpoint& target,
        Map& from,
        Map& to) noexcept -> bool
    {
        try {
            auto set = Map::iterator{from.end()};
            auto node = search(target, from, set);
            auto& dest = to[set->first];
            dest.insert(set->second.extract(node));

            return true;
        } catch (...) {

            return false;
        }
    }
    auto move_txos(
        const ot::blockchain::block::Txid& txid,
        Map& from,
        Map& to) noexcept -> std::size_t
    {
        auto toMove =
            std::pmr::vector<std::pair<Map::iterator, TXOSet::iterator>>{};

        for (auto s{from.begin()}; s != from.end(); ++s) {
            auto& set = s->second;

            for (auto t{set.begin()}; t != set.end(); ++t) {
                if (t->outpoint_.Txid() == txid.Bytes()) {
                    toMove.emplace_back(s, t);
                }
            }
        }

        for (auto& [set, node] : toMove) {
            auto& dest = to[set->first];
            dest.insert(set->second.extract(node));
        }

        return toMove.size();
    }
    auto search(
        const ot::blockchain::block::Outpoint& target,
        Map& map,
        Map::iterator& output) noexcept(false) -> TXOSet::iterator
    {
        for (auto s{map.begin()}; s != map.end(); ++s) {
            auto& value = s->second;

            for (auto t{value.begin()}; t != value.end(); ++t) {
                if (t->outpoint_ == target) {
                    output = s;

                    return t;
                }
            }
        }

        throw std::runtime_error{"not found"};
    }
    auto spend_txo(const ot::blockchain::block::Outpoint& txo, Map& to) noexcept
        -> bool
    {
        if (move_txo(txo, confirmed_incoming_, to)) {

            return true;
        } else if (move_txo(txo, unconfirmed_incoming_, to)) {

            return true;
        } else {

            return false;
        }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

TXOs::TXOs(const User& owner) noexcept
    : imp_(std::make_unique<Imp>(owner))
{
}

auto TXOs::AddConfirmed(
    const ot::blockchain::block::bitcoin::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
{
    return imp_->AddConfirmed(tx, index, owner);
}

auto TXOs::AddGenerated(
    const ot::blockchain::block::bitcoin::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner,
    const ot::blockchain::block::Height position) noexcept -> bool
{
    return imp_->AddGenerated(tx, index, owner, position);
}

auto TXOs::AddUnconfirmed(
    const ot::blockchain::block::bitcoin::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
{
    return imp_->AddUnconfirmed(tx, index, owner);
}

auto TXOs::Confirm(const ot::blockchain::block::Txid& transaction) noexcept
    -> bool
{
    return imp_->Confirm(transaction);
}

auto TXOs::Mature(const ot::blockchain::block::Height position) noexcept -> bool
{
    return imp_->Mature(position);
}

auto TXOs::Orphan(const ot::blockchain::block::Txid& transaction) noexcept
    -> bool
{
    return imp_->Orphan(transaction);
}

auto TXOs::OrphanGeneration(
    const ot::blockchain::block::Txid& transaction) noexcept -> bool
{
    return imp_->OrphanGeneration(transaction);
}

auto TXOs::SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
    -> bool
{
    return imp_->SpendUnconfirmed(txo);
}

auto TXOs::SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
    -> bool
{
    return imp_->SpendConfirmed(txo);
}

auto TXOs::Extract(TXOState& output) const noexcept -> void
{
    return imp_->Extract(output);
}

TXOs::~TXOs() = default;

struct WalletListener::Imp {
    const ot::api::Session& api_;
    mutable std::mutex lock_;
    std::promise<Height> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    Imp(const ot::api::Session& api) noexcept
        : api_(api)
        , lock_()
        , promise_()
        , target_(-1)
        , cb_(zmq::ListenCallback::Factory([&](zmq::Message&& msg) {
            const auto body = msg.Body();

            OT_ASSERT(body.size() == 4);

            auto lock = ot::Lock{lock_};
            const auto height = body.at(2).as<Height>();

            if (height == target_) {
                try {
                    promise_.set_value(height);
                } catch (...) {
                }
            }
        }))
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainSyncProgress()));
    }
};

WalletListener::WalletListener(const ot::api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto WalletListener::GetFuture(const Height height) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

WalletListener::~WalletListener() = default;

bool Regtest_fixture_base::init_{false};
Regtest_fixture_base::Expected Regtest_fixture_base::expected_{};
Regtest_fixture_base::Transactions Regtest_fixture_base::transactions_{};
ot::blockchain::block::Height Regtest_fixture_base::height_{0};
using TxoState = ot::blockchain::node::TxoState;
const std::pmr::set<TxoState> Regtest_fixture_base::states_{
    TxoState::UnconfirmedNew,
    TxoState::UnconfirmedSpend,
    TxoState::ConfirmedNew,
    TxoState::ConfirmedSpend,
    TxoState::All,
};
std::unique_ptr<const ot::OTBlockchainAddress>
    Regtest_fixture_base::listen_address_{};
std::unique_ptr<const PeerListener> Regtest_fixture_base::peer_listener_{};
std::unique_ptr<MinedBlocks> Regtest_fixture_base::mined_block_cache_{};
Regtest_fixture_base::BlockListen Regtest_fixture_base::block_listener_{};
Regtest_fixture_base::WalletListen Regtest_fixture_base::wallet_listener_{};
const User Regtest_fixture_hd::alice_{GetVectors3().alice_.words_, "Alice"};
TXOs Regtest_fixture_hd::txos_{alice_};
std::unique_ptr<ScanListener> Regtest_fixture_hd::listener_p_{};
const User Regtest_fixture_sync::alex_{GetVectors3().alice_.words_, "Alex"};
std::optional<ot::OTServerContract> Regtest_fixture_sync::notary_{std::nullopt};
std::optional<ot::OTUnitDefinition> Regtest_fixture_sync::unit_{std::nullopt};
std::unique_ptr<SyncSubscriber> Regtest_fixture_sync::sync_subscriber_{};
std::unique_ptr<SyncRequestor> Regtest_fixture_sync::sync_requestor_{};
Server Regtest_payment_code::server_1_{};
const User Regtest_payment_code::alice_{GetVectors3().alice_.words_, "Alice"};
const User Regtest_payment_code::bob_{GetVectors3().bob_.words_, "Bob"};
TXOs Regtest_payment_code::txos_alice_{alice_};
TXOs Regtest_payment_code::txos_bob_{bob_};
std::unique_ptr<ScanListener> Regtest_payment_code::listener_alice_p_{};
std::unique_ptr<ScanListener> Regtest_payment_code::listener_bob_p_{};
}  // namespace ottest
