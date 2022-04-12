// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "blockchain/node/base/Base.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <iomanip>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "blockchain/node/base/SyncServer.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"  // IWYU pragma: keep
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/p2p/Requestor.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/PushTransaction.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposedNotification.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposedOutput.pb.h"
#include "serialization/protobuf/HDPath.pb.h"
#include "serialization/protobuf/PaymentCode.pb.h"

namespace opentxs::blockchain::node::implementation
{
constexpr auto proposal_version_ = VersionNumber{1};
constexpr auto notification_version_ = VersionNumber{1};
constexpr auto output_version_ = VersionNumber{1};

struct NullWallet final : public node::internal::Wallet {
    const api::Session& api_;

    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal&,
        std::promise<SendOutcome>&& promise) const noexcept -> void final
    {
        static const auto blank = api_.Factory().Data();
        promise.set_value({SendResult::UnspecifiedError, blank});
    }
    auto FeeEstimate() const noexcept -> std::optional<Amount> final
    {
        return {};
    }
    auto GetBalance() const noexcept -> Balance final { return {}; }
    auto GetBalance(const identifier::Nym&) const noexcept -> Balance final
    {
        return {};
    }
    auto GetBalance(const identifier::Nym&, const Identifier&) const noexcept
        -> Balance final
    {
        return {};
    }
    auto GetBalance(const crypto::Key&) const noexcept -> Balance final
    {
        return {};
    }
    auto GetOutputs(alloc::Resource* alloc) const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(TxoState, alloc::Resource* alloc) const noexcept
        -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(const identifier::Nym&, alloc::Resource* alloc)
        const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(const identifier::Nym&, TxoState, alloc::Resource* alloc)
        const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(
        const identifier::Nym&,
        const Identifier&,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(
        const identifier::Nym&,
        const Identifier&,
        TxoState,
        alloc::Resource* alloc) const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetOutputs(const crypto::Key&, TxoState, alloc::Resource* alloc)
        const noexcept -> Vector<UTXO> final
    {
        return Vector<UTXO>{alloc};
    }
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final
    {
        return {};
    }
    auto Height() const noexcept -> block::Height final { return {}; }
    auto Init() noexcept -> void final {}
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        auto promise = std::promise<void>{};
        promise.set_value();

        return promise.get_future();
    }

    NullWallet(const api::Session& api)
        : api_(api)
    {
    }
    NullWallet(const NullWallet&) = delete;
    NullWallet(NullWallet&&) = delete;
    auto operator=(const NullWallet&) -> NullWallet& = delete;
    auto operator=(NullWallet&&) -> NullWallet& = delete;

    ~NullWallet() final = default;
};

Base::Base(
    const api::Session& api,
    const Type type,
    const node::internal::Config& config,
    const UnallocatedCString& seednode,
    const UnallocatedCString& syncEndpoint) noexcept
    : Worker(api, 0s)
    , chain_(type)
    , filter_type_([&] {
        if (config.generate_cfilters_ || config.use_sync_server_) {

            return cfilter::Type::ES;
        }

        return blockchain::internal::DefaultFilter(chain_);
    }())
    , shutdown_sender_(
          api.Network().ZeroMQ(),
          network::zeromq::MakeArbitraryInproc())
    , database_p_(factory::BlockchainDatabase(
          api,
          *this,
          api_.Network().Blockchain().Internal().Database(),
          chain_,
          filter_type_))
    , config_(config)
    , mempool_(
          api_.Crypto().Blockchain(),
          *database_p_,
          api_.Network().Blockchain().Internal().Mempool(),
          chain_)
    , header_p_(factory::HeaderOracle(api, *database_p_, chain_))
    , block_(factory::BlockOracle(
          api,
          *this,
          *header_p_,
          *database_p_,
          chain_,
          shutdown_sender_.endpoint_))
    , filter_p_(factory::BlockchainFilterOracle(
          api,
          config_,
          *this,
          *header_p_,
          block_,
          *database_p_,
          chain_,
          filter_type_,
          shutdown_sender_.endpoint_))
    , peer_p_(factory::BlockchainPeerManager(
          api,
          config_,
          mempool_,
          *this,
          *header_p_,
          *filter_p_,
          block_,
          *database_p_,
          chain_,
          database_p_->BlockPolicy(),
          seednode,
          shutdown_sender_.endpoint_))
    , wallet_p_([&]() -> std::unique_ptr<blockchain::node::internal::Wallet> {
        if (config_.disable_wallet_) {

            return std::make_unique<NullWallet>(api);
        } else {
            return factory::BlockchainWallet(
                api,
                *this,
                *database_p_,
                mempool_,
                chain_,
                shutdown_sender_.endpoint_);
        }
    }())
    , database_(*database_p_)
    , filters_(*filter_p_)
    , header_(*header_p_)
    , peer_(*peer_p_)
    , wallet_(*wallet_p_)
    , start_(Clock::now())
    , sync_endpoint_(syncEndpoint)
    , requestor_endpoint_(network::zeromq::MakeArbitraryInproc())
    , sync_server_([&] {
        if (config_.provide_sync_server_) {
            return std::make_unique<base::SyncServer>(
                api_,
                database_,
                header_,
                filters_,
                *this,
                chain_,
                filters_.DefaultType(),
                shutdown_sender_.endpoint_,
                sync_endpoint_);
        } else {
            return std::unique_ptr<base::SyncServer>{};
        }
    }())
    , p2p_requestor_([&] {
        if (config_.use_sync_server_) {

            return std::make_unique<p2p::Requestor>(
                api_, chain_, requestor_endpoint_);
        } else {

            return std::unique_ptr<p2p::Requestor>{};
        }
    }())
    , sync_cb_(zmq::ListenCallback::Factory(
          [&](auto&& m) { pipeline_.Push(std::move(m)); }))
    , sync_socket_(
          api_.Network().ZeroMQ().PairSocket(sync_cb_, requestor_endpoint_))
    , local_chain_height_(0)
    , remote_chain_height_(params::Chains().at(chain_).checkpoint_.height_)
    , waiting_for_headers_(Flag::Factory(false))
    , headers_requested_(Clock::now())
    , headers_received_()
    , work_promises_()
    , send_promises_()
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
    , header_sync_()
    , filter_sync_()
    , state_(State::UpdatingHeaders)
    , init_promise_()
    , init_(init_promise_.get_future())
{
    OT_ASSERT(database_p_);
    OT_ASSERT(filter_p_);
    OT_ASSERT(header_p_);
    OT_ASSERT(peer_p_);
    OT_ASSERT(wallet_p_);

    header_.Internal().Init();
    init_executor({UnallocatedCString{
        api_.Endpoints().Internal().BlockchainFilterUpdated(chain_)}});
    LogVerbose()(config_.print()).Flush();

    for (const auto& addr : api_.GetOptions().BlockchainBindIpv4()) {
        try {
            const auto boost = boost::asio::ip::make_address(addr);

            if (false == boost.is_v4()) {
                throw std::runtime_error{"Wrong address type (not ipv4)"};
            }

            auto address = opentxs::factory::BlockchainAddress(
                api_,
                blockchain::p2p::Protocol::bitcoin,
                blockchain::p2p::Network::ipv4,
                [&] {
                    auto out = api_.Factory().Data();
                    const auto v4 = boost.to_v4();
                    const auto bytes = v4.to_bytes();
                    out->Assign(bytes.data(), bytes.size());

                    return out;
                }(),
                params::Chains().at(chain_).default_port_,
                chain_,
                {},
                {},
                false);

            if (!address) { continue; }

            peer_.Listen(*address);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }

    for (const auto& addr : api_.GetOptions().BlockchainBindIpv6()) {
        try {
            const auto boost = boost::asio::ip::make_address(addr);

            if (false == boost.is_v6()) {
                throw std::runtime_error{"Wrong address type (not ipv6)"};
            }

            auto address = opentxs::factory::BlockchainAddress(
                api_,
                blockchain::p2p::Protocol::bitcoin,
                blockchain::p2p::Network::ipv6,
                [&] {
                    auto out = api_.Factory().Data();
                    const auto v6 = boost.to_v6();
                    const auto bytes = v6.to_bytes();
                    out->Assign(bytes.data(), bytes.size());

                    return out;
                }(),
                params::Chains().at(chain_).default_port_,
                chain_,
                {},
                {},
                false);

            if (!address) { continue; }

            peer_.Listen(*address);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }
}

auto Base::AddBlock(const std::shared_ptr<const block::bitcoin::Block> pBlock)
    const noexcept -> bool
{
    if (!pBlock) {
        LogError()(OT_PRETTY_CLASS())("invalid ")(print(chain_))(" block")
            .Flush();

        return false;
    }

    const auto& block = *pBlock;

    try {
        const auto bytes = [&] {
            auto output = Space{};

            if (false == block.Serialize(writer(output))) {
                throw std::runtime_error("Serialization error");
            }

            return output;
        }();
        block_.SubmitBlock(reader(bytes));
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("failed to serialize ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    const auto& id = block.ID();

    if (std::future_status::ready != block_.LoadBitcoin(id).wait_for(60s)) {
        LogError()(OT_PRETTY_CLASS())("failed to load ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    if (false == filters_.ProcessBlock(block)) {
        LogError()(OT_PRETTY_CLASS())("failed to index ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    if (false == header_.AddHeader(block.Header().clone())) {
        LogError()(OT_PRETTY_CLASS())("failed to process ")(print(chain_))(
            " header")
            .Flush();

        return false;
    }

    return peer_.BroadcastBlock(block);
}

auto Base::AddPeer(const blockchain::p2p::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.AddPeer(address);
}

auto Base::BroadcastTransaction(
    const block::bitcoin::Transaction& tx,
    const bool pushtx) const noexcept -> bool
{
    mempool_.Submit(tx.clone());

    if (pushtx && p2p_requestor_) {
        sync_socket_->Send([&] {
            auto out = network::zeromq::Message{};
            const auto command =
                factory::BlockchainSyncPushTransaction(chain_, tx);
            command.Serialize(out);

            return out;
        }());
    }

    if (false == running_.load()) { return false; }

    // TODO upgrade mempool logic so this becomes unnecessary

    return peer_.BroadcastTransaction(tx);
}

auto Base::Connect() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.Connect();
}

auto Base::Disconnect() noexcept -> bool
{
    // TODO

    return false;
}

auto Base::FeeRate() const noexcept -> Amount
{
    // TODO in full node mode, calculate the fee network from the mempool and
    // recent blocks
    // TODO on networks that support it, query the fee rate from network peers
    const auto http = wallet_.FeeEstimate();
    const auto fallback = params::Chains().at(chain_).default_fee_rate_;
    const auto chain = print(chain_);
    LogConsole()(chain)(" defined minimum fee rate is: ")(fallback).Flush();

    if (http.has_value()) {
        LogConsole()(chain)(" transaction fee rate via https oracle is: ")(
            http.value())
            .Flush();
    } else {
        LogConsole()(chain)(
            " transaction fee estimates via https oracle not available")
            .Flush();
    }

    auto out = std::max<Amount>(fallback, http.value_or(0));
    LogConsole()("Using ")(out)(" for current ")(chain)(" fee rate").Flush();

    return out;
}

auto Base::GetConfirmations(const UnallocatedCString& txid) const noexcept
    -> ChainHeight
{
    // TODO

    return -1;
}

auto Base::GetPeerCount() const noexcept -> std::size_t
{
    if (false == running_.load()) { return 0; }

    return peer_.GetPeerCount();
}

auto Base::GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>
{
    return database_.GetTransactions();
}

auto Base::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::pTxid>
{
    return database_.GetTransactions(account);
}

auto Base::GetVerifiedPeerCount() const noexcept -> std::size_t
{
    if (false == running_.load()) { return 0; }

    return peer_.GetVerifiedPeerCount();
}

auto Base::init() noexcept -> void
{
    local_chain_height_.store(header_.BestChain().first);

    {
        const auto best = database_.CurrentBest();

        OT_ASSERT(best);

        const auto position = best->Position();
        LogVerbose()(print(chain_))(" chain initialized with best hash ")(
            print(position))
            .Flush();
    }

    peer_.init();
    notify_sync_client();
    init_promise_.set_value();
    trigger();
    reset_heartbeat();
}

auto Base::IsWalletScanEnabled() const noexcept -> bool
{
    switch (state_.load()) {
        case State::UpdatingHeaders:
        case State::UpdatingBlocks:
        case State::UpdatingFilters:
        case State::UpdatingSyncData:
        case State::Normal: {

            return true;
        }
        default: {

            return false;
        }
    }
}

auto Base::is_synchronized_blocks() const noexcept -> bool
{
    return block_.Tip().first >= this->target();
}

auto Base::is_synchronized_filters() const noexcept -> bool
{
    const auto target = this->target();
    const auto progress = filters_.Tip(filters_.DefaultType()).first;

    return (progress >= target);
}

auto Base::is_synchronized_headers() const noexcept -> bool
{
    const auto target = remote_chain_height_.load();
    const auto progress = local_chain_height_.load();

    return (progress >= target);
}

auto Base::is_synchronized_sync_server() const noexcept -> bool
{
    if (sync_server_) {

        return sync_server_->Tip().first >= this->target();
    } else {

        return false;
    }
}

auto Base::JobReady(const node::internal::PeerManager::Task type) const noexcept
    -> void
{
    if (peer_p_) { peer_.JobReady(type); }
}

auto Base::Listen(const blockchain::p2p::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.Listen(address);
}

auto Base::notify_sync_client() const noexcept -> void
{
    if (p2p_requestor_) {
        sync_socket_->Send([this] {
            const auto tip = filters_.FilterTip(filters_.DefaultType());
            auto msg = MakeWork(OTZMQWorkType{OT_ZMQ_INTERNAL_SIGNAL + 2});
            msg.AddFrame(tip.first);
            msg.AddFrame(tip.second);

            return msg;
        }());
    }
}

auto Base::PeerTarget() const noexcept -> std::size_t
{
    return peer_.PeerTarget();
}

auto Base::pipeline(zmq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    init_.get();
    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body.at(0).as<Task>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            OT_FAIL;
        }
    }();

    switch (task) {
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Task::SyncReply:
        case Task::SyncNewBlock: {
            process_sync_data(std::move(in));
        } break;
        case Task::SubmitBlockHeader: {
            process_header(std::move(in));
            do_work();
        } break;
        case Task::SubmitBlock: {
            process_block(std::move(in));
        } break;
        case Task::Heartbeat: {
            mempool_.Heartbeat();
            block_.Heartbeat();
            filters_.Heartbeat();
            peer_.Heartbeat();

            if (sync_server_) { sync_server_->Heartbeat(); }

            do_work();
            reset_heartbeat();
        } break;
        case Task::SendToAddress: {
            process_send_to_address(std::move(in));
        } break;
        case Task::SendToPaymentCode: {
            process_send_to_payment_code(std::move(in));
        } break;
        case Task::StartWallet: {
            wallet_.Init();
        } break;
        case Task::FilterUpdate: {
            process_filter_update(std::move(in));
        } break;
        case Task::StateMachine: {
            do_work();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Base::process_block(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (2 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid block").Flush();

        return;
    }

    block_.SubmitBlock(body.at(1).Bytes());
}

auto Base::process_filter_update(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto height = body.at(2).as<block::Height>();
    const auto target = this->target();

    {
        const auto progress =
            (0 == target) ? double{0}
                          : ((double(height) / double(target)) * double{100});
        auto display = std::stringstream{};
        display << std::setprecision(3) << progress << "%";

        if (false == config_.disable_wallet_) {
            LogDetail()(print(chain_))(" chain sync progress: ")(
                height)(" of ")(target)(" (")(display.str())(")")
                .Flush();
        }
    }

    api_.Network().Blockchain().Internal().ReportProgress(
        chain_, height, target);
}

auto Base::process_header(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    waiting_for_headers_->Off();
    headers_received_ = Clock::now();
    auto promise = int{};
    auto input = UnallocatedVector<ReadView>{};

    {
        const auto body = in.Body();

        if (2 > body.size()) {
            LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

            return;
        }

        // NOTE can not use std::prev on frame iterators
        auto end = std::next(body.begin(), body.size() - 1u);
        std::transform(
            std::next(body.begin()),
            end,
            std::back_inserter(input),
            [](const auto& frame) { return frame.Bytes(); });
        const auto& promiseFrame = *end;
        promise = promiseFrame.as<int>();
    }

    auto headers = UnallocatedVector<std::unique_ptr<block::Header>>{};

    for (const auto& header : input) {
        headers.emplace_back(instantiate_header(header));
    }

    if (false == headers.empty()) { header_.AddHeaders(headers); }

    work_promises_.clear(promise);
}

auto Base::process_send_to_address(network::zeromq::Message&& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto sender = api_.Factory().NymID(body.at(1));
    const auto address = UnallocatedCString{body.at(2).Bytes()};
    const auto amount = factory::Amount(body.at(3));
    const auto memo = UnallocatedCString{body.at(4).Bytes()};
    const auto promise = body.at(5).as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto pNym = api_.Wallet().Nym(sender);

        if (!pNym) {
            const auto error =
                UnallocatedCString{"Invalid sender "} + sender->str();
            rc = SendResult::InvalidSenderNym;

            throw std::runtime_error{error};
        }

        const auto [data, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(address);

        if ((0 == chains.count(chain_)) || (!supported)) {
            using namespace std::literals;
            const auto error = CString{"Address "}
                                   .append(address)
                                   .append(" not valid for "sv)
                                   .append(blockchain::print(chain_));
            rc = SendResult::AddressNotValidforChain;

            throw std::runtime_error{error.c_str()};
        }

        auto id = api_.Factory().Identifier();
        id->Randomize();
        auto proposal = proto::BlockchainTransactionProposal{};
        proposal.set_version(proposal_version_);
        proposal.set_id(id->str());
        proposal.set_initiator(sender->data(), sender->size());
        proposal.set_expires(
            Clock::to_time_t(Clock::now() + std::chrono::hours(1)));
        proposal.set_memo(memo);
        using Style = blockchain::crypto::AddressStyle;
        auto& output = *proposal.add_output();
        output.set_version(output_version_);
        amount.Serialize(writer(output.mutable_amount()));

        switch (style) {
            case Style::P2WPKH: {
                output.set_segwit(true);
                [[fallthrough]];
            }
            case Style::P2PKH: {
                output.set_pubkeyhash(data->str());
            } break;
            case Style::P2WSH: {
                output.set_segwit(true);
                [[fallthrough]];
            }
            case Style::P2SH: {
                output.set_scripthash(data->str());
            } break;
            default: {
                rc = SendResult::UnsupportedAddressFormat;

                throw std::runtime_error{"Unsupported address type"};
            }
        }

        wallet_.ConstructTransaction(proposal, send_promises_.finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        static const auto blank = api_.Factory().Data();
        send_promises_.finish(promise).set_value({rc, blank});
    }
}

auto Base::process_send_to_payment_code(network::zeromq::Message&& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto nymID = api_.Factory().NymID(body.at(1));
    const auto recipient =
        api_.Factory().PaymentCode(UnallocatedCString{body.at(2).Bytes()});
    const auto contact =
        api_.Crypto().Blockchain().Internal().Contacts().PaymentCodeToContact(
            recipient, chain_);
    const auto amount = factory::Amount(body.at(3));
    const auto memo = UnallocatedCString{body.at(4).Bytes()};
    const auto promise = body.at(5).as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto pNym = api_.Wallet().Nym(nymID);

        if (!pNym) {
            rc = SendResult::InvalidSenderNym;

            throw std::runtime_error{
                UnallocatedCString{"Unable to load recipient nym ("} +
                nymID->str() + ')'};
        }

        const auto& nym = *pNym;
        const auto sender = api_.Factory().PaymentCode(nym.PaymentCode());

        if (0 == sender.Version()) {
            rc = SendResult::SenderMissingPaymentCode;

            throw std::runtime_error{"Invalid sender payment code"};
        }

        if (3 > recipient.Version()) {
            rc = SendResult::UnsupportedRecipientPaymentCode;

            throw std::runtime_error{
                "Sending to version 1 payment codes not yet supported"};
        }

        const auto path = [&] {
            auto out = proto::HDPath{};

            if (false == nym.Internal().PaymentCodePath(out)) {
                rc = SendResult::HDDerivationFailure;

                throw std::runtime_error{
                    "Failed to obtain payment code HD path"};
            }

            return out;
        }();
        const auto reason = api_.Factory().PasswordPrompt(
            UnallocatedCString{"Sending a transaction to "} +
            recipient.asBase58());
        const auto& account =
            api_.Crypto().Blockchain().Internal().PaymentCodeSubaccount(
                nymID, sender, recipient, path, chain_, reason);
        using Subchain = blockchain::crypto::Subchain;
        constexpr auto subchain{Subchain::Outgoing};
        const auto index = account.Reserve(subchain, reason);

        if (false == index.has_value()) {
            rc = SendResult::HDDerivationFailure;

            throw std::runtime_error{"Failed to allocate next key"};
        }

        const auto pKey = [&] {
            const auto& element =
                account.BalanceElement(subchain, index.value());
            auto out = element.Key();

            if (!out) {
                rc = SendResult::HDDerivationFailure;

                throw std::runtime_error{"Failed to instantiate key"};
            }

            return out;
        }();
        const auto& key = *pKey;
        const auto proposal = [&] {
            auto out = proto::BlockchainTransactionProposal{};
            out.set_version(proposal_version_);
            out.set_id([&] {
                auto id = api_.Factory().Identifier();
                id->Randomize();

                return id->str();
            }());
            out.set_initiator(nymID->data(), nymID->size());
            out.set_expires(
                Clock::to_time_t(Clock::now() + std::chrono::hours(1)));
            out.set_memo(memo);
            auto& txout = *out.add_output();
            txout.set_version(output_version_);
            amount.Serialize(writer(txout.mutable_amount()));
            txout.set_index(index.value());
            txout.set_paymentcodechannel(account.ID().str());
            const auto pubkey = api_.Factory().DataFromBytes(key.PublicKey());
            LogVerbose()(OT_PRETTY_CLASS())(" using derived public key ")(
                pubkey->asHex())(
                " at "
                "index"
                " ")(index.value())(" for outgoing transaction")
                .Flush();
            txout.set_pubkey(pubkey->str());
            txout.set_contact(UnallocatedCString{contact->Bytes()});

            if (account.IsNotified()) {
                // TODO preemptive notifications go here
            } else {
                auto serialize =
                    [&](const PaymentCode& pc) -> proto::PaymentCode {
                    auto proto = proto::PaymentCode{};
                    pc.Internal().Serialize(proto);
                    return proto;
                };
                auto& notif = *out.add_notification();
                notif.set_version(notification_version_);
                *notif.mutable_sender() = serialize(sender);
                *notif.mutable_path() = path;
                *notif.mutable_recipient() = serialize(recipient);
            }

            return out;
        }();

        wallet_.ConstructTransaction(proposal, send_promises_.finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        static const auto blank = api_.Factory().Data();
        send_promises_.finish(promise).set_value({rc, blank});
    }
}

auto Base::process_sync_data(network::zeromq::Message&& in) noexcept -> void
{
    const auto start = Clock::now();
    const auto sync = api_.Factory().BlockchainSyncMessage(in);
    const auto& data = sync->asData();

    {
        const auto& state = data.State();

        if (state.Chain() != chain_) {
            LogError()(OT_PRETTY_CLASS())("Wrong chain").Flush();

            return;
        }

        remote_chain_height_.store(
            std::max(state.Position().first, remote_chain_height_.load()));
    }

    auto prior = block::Hash{};
    auto hashes = Vector<block::Hash>{};
    const auto accepted =
        header_.Internal().ProcessSyncData(prior, hashes, data);

    if (0u < accepted) {
        const auto& blocks = data.Blocks();

        LogVerbose()("Accepted ")(accepted)(" of ")(blocks.size())(" ")(
            print(chain_))(" headers")
            .Flush();
        filters_.ProcessSyncData(prior, hashes, data);
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                Clock::now() - start);
        LogDetail()("Processed ")(blocks.size())(" ")(print(chain_))(
            " sync packets in ")(elapsed)
            .Flush();
    } else {
        LogVerbose()("Invalid ")(print(chain_))(" sync data").Flush();
    }

    notify_sync_client();
}

auto Base::Reorg() const noexcept -> const network::zeromq::socket::Publish&
{
    return api_.Network().Blockchain().Internal().Reorg();
}

auto Base::RequestBlock(const block::Hash& block) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.RequestBlock(block);
}

auto Base::RequestBlocks(
    const UnallocatedVector<ReadView>& hashes) const noexcept -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.RequestBlocks(hashes);
}

auto Base::reset_heartbeat() noexcept -> void
{
    static constexpr auto interval = 5s;
    heartbeat_.SetRelative(interval);
    heartbeat_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            pipeline_.Push(MakeWork(Task::Heartbeat));
        }
    });
}

auto Base::SendToAddress(
    const opentxs::identifier::Nym& sender,
    const UnallocatedCString& address,
    const Amount amount,
    const UnallocatedCString& memo) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    auto work = MakeWork(Task::SendToAddress);
    work.AddFrame(sender);
    work.AddFrame(address);
    amount.Serialize(work.AppendBytes());
    work.AddFrame(memo);
    work.AddFrame(index);
    pipeline_.Push(std::move(work));

    return std::move(future);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    const UnallocatedCString& recipient,
    const Amount amount,
    const UnallocatedCString& memo) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    auto work = MakeWork(Task::SendToPaymentCode);
    work.AddFrame(nymID);
    work.AddFrame(recipient);
    amount.Serialize(work.AppendBytes());
    work.AddFrame(memo);
    work.AddFrame(index);
    pipeline_.Push(std::move(work));

    return std::move(future);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    const PaymentCode& recipient,
    const Amount amount,
    const UnallocatedCString& memo) const noexcept -> PendingOutgoing
{
    return SendToPaymentCode(nymID, recipient.asBase58(), amount, memo);
}

auto Base::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        init_.get();
        pipeline_.Close();
        shutdown_sender_.Activate();
        wallet_.Shutdown();

        if (sync_server_) { sync_server_->Shutdown(); }

        if (p2p_requestor_) {
            sync_socket_->Send(MakeWork(WorkType::Shutdown));
        }

        peer_.Shutdown();
        filters_.Shutdown();
        block_.Shutdown();
        shutdown_sender_.Close();
        promise.set_value();
    }
}

auto Base::StartWallet() noexcept -> void
{
    pipeline_.Push(MakeWork(Task::StartWallet));
}

auto Base::state_machine() noexcept -> bool
{
    if (false == running_.load()) { return false; }

    const auto& log = LogTrace();
    log(OT_PRETTY_CLASS())("Starting state machine for ")(print(chain_))
        .Flush();
    state_machine_headers();

    switch (state_.load()) {
        case State::UpdatingHeaders: {
            if (is_synchronized_headers()) {
                header_sync_ = Clock::now();
                const auto interval =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        header_sync_ - start_);
                LogConsole()(print(chain_))(
                    " block header chain synchronized in ")(interval)
                    .Flush();
                using Policy = database::BlockStorage;

                if (Policy::All == database_.BlockPolicy()) {
                    state_transition_blocks();
                } else {
                    state_transition_filters();
                }
            } else {
                log(OT_PRETTY_CLASS())("updating ")(print(chain_))(
                    " header oracle")
                    .Flush();
            }
        } break;
        case State::UpdatingBlocks: {
            if (is_synchronized_blocks()) {
                log(OT_PRETTY_CLASS())(print(chain_))(
                    " block oracle is synchronized")
                    .Flush();
                state_transition_filters();
            } else {
                log(OT_PRETTY_CLASS())("updating ")(print(chain_))(
                    " block oracle")
                    .Flush();

                break;
            }
        } break;
        case State::UpdatingFilters: {
            if (is_synchronized_filters()) {
                filter_sync_ = Clock::now();
                const auto interval =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        filter_sync_ - start_);
                LogConsole()(print(chain_))(" cfilter chain synchronized in ")(
                    interval)
                    .Flush();

                if (config_.provide_sync_server_) {
                    state_transition_sync();
                } else {
                    state_transition_normal();
                }
            } else {
                log(OT_PRETTY_CLASS())("updating ")(print(chain_))(
                    " filter oracle")
                    .Flush();

                break;
            }
        } break;
        case State::UpdatingSyncData: {
            if (is_synchronized_sync_server()) {
                log(OT_PRETTY_CLASS())(print(chain_))(
                    " sync server is synchronized")
                    .Flush();
                state_transition_normal();
            } else {
                log(OT_PRETTY_CLASS())("updating ")(print(chain_))(
                    " sync server")
                    .Flush();

                break;
            }
        } break;
        case State::Normal:
        default: {
        }
    }

    log(OT_PRETTY_CLASS())("Completed state machine for ")(print(chain_))
        .Flush();

    return false;
}

auto Base::state_machine_headers() noexcept -> void
{
    constexpr auto limit = std::chrono::minutes{5};
    constexpr auto timeout = 30s;
    constexpr auto rateLimit = 1s;
    const auto requestInterval = Clock::now() - headers_requested_;
    const auto receiveInterval = Clock::now() - headers_received_;
    const auto requestHeaders = [&] {
        LogVerbose()(OT_PRETTY_CLASS())("Requesting ")(print(chain_))(
            " block headers from all connected peers "
            "(instance ")(api_.Instance())(")")
            .Flush();
        waiting_for_headers_->On();
        peer_.RequestHeaders();
        headers_requested_ = Clock::now();
    };

    if (requestInterval < rateLimit) { return; }

    if (waiting_for_headers_.get()) {
        if (requestInterval < timeout) { return; }

        LogDetail()(OT_PRETTY_CLASS())(print(chain_))(
            " headers not received before timeout "
            "(instance ")(api_.Instance())(")")
            .Flush();
        requestHeaders();
    } else if ((!is_synchronized_headers()) && (!config_.use_sync_server_)) {
        requestHeaders();
    } else if (receiveInterval >= limit) {
        requestHeaders();
    }
}

auto Base::state_transition_blocks() noexcept -> void
{
    block_.Init();
    state_.store(State::UpdatingBlocks);
}

auto Base::state_transition_filters() noexcept -> void
{
    filters_.Start();
    state_.store(State::UpdatingFilters);
}

auto Base::state_transition_normal() noexcept -> void
{
    state_.store(State::Normal);
}

auto Base::state_transition_sync() noexcept -> void
{
    OT_ASSERT(sync_server_);

    sync_server_->Start();
    state_.store(State::UpdatingSyncData);
}

auto Base::Submit(network::zeromq::Message&& work) const noexcept -> void
{
    if (false == running_.load()) { return; }

    pipeline_.Push(std::move(work));
}

auto Base::SyncTip() const noexcept -> block::Position
{
    static const auto blank = make_blank<block::Position>::value(api_);

    if (sync_server_) {

        return sync_server_->Tip();
    } else {

        return blank;
    }
}

auto Base::Track(network::zeromq::Message&& work) const noexcept
    -> std::future<void>
{
    if (false == running_.load()) {
        auto promise = std::promise<void>{};
        promise.set_value();

        return promise.get_future();
    }

    auto [index, future] = work_promises_.get();
    work.AddFrame(index);
    pipeline_.Push(std::move(work));

    return std::move(future);
}

auto Base::target() const noexcept -> block::Height
{
    return std::max(local_chain_height_.load(), remote_chain_height_.load());
}

auto Base::UpdateHeight(const block::Height height) const noexcept -> void
{
    if (false == running_.load()) { return; }

    remote_chain_height_.store(std::max(height, remote_chain_height_.load()));
    trigger();
}

auto Base::UpdateLocalHeight(const block::Position position) const noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto& [height, hash] = position;
    LogDetail()(print(chain_))(" block header chain updated to hash ")(
        print(position))
        .Flush();
    local_chain_height_.store(height);
    trigger();
}

Base::~Base() { Shutdown().get(); }
}  // namespace opentxs::blockchain::node::implementation
