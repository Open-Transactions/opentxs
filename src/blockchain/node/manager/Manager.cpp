// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "blockchain/node/manager/Manager.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <BlockchainTransactionProposedNotification.pb.h>
#include <BlockchainTransactionProposedOutput.pb.h>
#include <HDPath.pb.h>
#include <PaymentCode.pb.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Factory.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/PeerManager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
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
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::implementation
{
constexpr auto proposal_version_ = VersionNumber{1};
constexpr auto notification_version_ = VersionNumber{1};
constexpr auto output_version_ = VersionNumber{1};

Base::Base(
    const api::Session& api,
    const Type type,
    const node::internal::Config& config,
    std::string_view seednode,
    node::Endpoints endpoints) noexcept
    : Worker(
          api,
          0s,
          "blockchain::node::Manager",
          {},
          {
              {endpoints.manager_pull_,
               network::zeromq::socket::Direction::Bind},
          },
          {},
          {
              {network::zeromq::socket::Type::Push,
               {
                   {endpoints.block_oracle_pull_,
                    network::zeromq::socket::Direction::Connect},
               }},
              {network::zeromq::socket::Type::Push,
               {
                   {endpoints.block_cache_pull_,
                    network::zeromq::socket::Direction::Connect},
               }},
              {network::zeromq::socket::Type::Push,
               {
                   {endpoints.wallet_pull_,
                    network::zeromq::socket::Direction::Connect},
               }},
              {network::zeromq::socket::Type::Push,
               {
                   {endpoints.otdht_pull_,
                    network::zeromq::socket::Direction::Connect},
               }},
              {network::zeromq::socket::Type::Push,
               {
                   {CString{api.Endpoints()
                                .Internal()
                                .Internal()
                                .BlockchainMessageRouter()},
                    network::zeromq::socket::Direction::Connect},
               }},
          })
    , chain_(type)
    , config_(config)
    , endpoints_(std::move(endpoints))
    , filter_type_([&] {
        switch (config_.profile_) {
            case BlockchainProfile::mobile:
            case BlockchainProfile::desktop: {

                return cfilter::Type::ES;
            }
            case BlockchainProfile::desktop_native:
            case BlockchainProfile::server: {

                return blockchain::internal::DefaultFilter(chain_);
            }
            default: {
                OT_FAIL;
            }
        }
    }())
    , shutdown_sender_(
          api.Network().Asio(),
          api.Network().ZeroMQ(),
          endpoints_.shutdown_publish_,
          CString{print(chain_)}
              .append(" on api instance ")
              .append(std::to_string(api_.Instance())))
    , database_p_(factory::BlockchainDatabase(
          api,
          *this,
          api_.Network().Blockchain().Internal().Database(),
          chain_,
          filter_type_))
    , mempool_(api_, api_.Crypto().Blockchain(), chain_, *database_p_)
    , header_(factory::HeaderOracle(api, *this))
    , block_()
    , filter_p_(factory::BlockchainFilterOracle(api, *this, filter_type_))
    , peer_p_(factory::BlockchainPeerManager(
          api,
          config_,
          mempool_,
          *this,
          header_,
          *filter_p_,
          block_,
          *database_p_,
          chain_,
          seednode,
          endpoints_))
    , database_(*database_p_)
    , filters_(*filter_p_)
    , peer_(*peer_p_)
    , wallet_()
    , to_block_oracle_(pipeline_.Internal().ExtraSocket(0))
    , to_block_cache_(pipeline_.Internal().ExtraSocket(1))
    , to_wallet_(pipeline_.Internal().ExtraSocket(2))
    , to_dht_(pipeline_.Internal().ExtraSocket(3))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(4))
    , send_promises_()
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
    , init_promise_()
    , init_(init_promise_.get_future())
    , self_()
{
    OT_ASSERT(database_p_);
    OT_ASSERT(filter_p_);
    OT_ASSERT(peer_p_);

    header_.Internal().Init();
    init_executor({UnallocatedCString{endpoints_.new_filter_publish_}});
    LogVerbose()(config_.Print()).Flush();  // TODO allocator

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
                    out.Assign(bytes.data(), bytes.size());

                    return out;
                }(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {},
                false);
            peer_.Listen(address);
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
                    out.Assign(bytes.data(), bytes.size());

                    return out;
                }(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {},
                false);
            peer_.Listen(address);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }
}

Base::Base(
    const api::Session& api,
    const Type type,
    const node::internal::Config& config,
    std::string_view seednode) noexcept
    : Base(
          api,
          type,
          config,
          seednode,
          alloc::Default{}  // TODO allocator
      )
{
}

auto Base::AddBlock(const std::shared_ptr<const bitcoin::block::Block> pBlock)
    const noexcept -> bool
{
    if (!pBlock) {
        LogError()(OT_PRETTY_CLASS())("invalid ")(print(chain_))(" block")
            .Flush();

        return false;
    }

    const auto& block = *pBlock;
    const auto& id = block.ID();

    if (false == block_.SubmitBlock(pBlock)) {
        LogError()(OT_PRETTY_CLASS())("failed to save ")(print(chain_))(
            " block ")
            .asHex(id)
            .Flush();

        return false;
    }

    if (false == filters_.Internal().ProcessBlock(block)) {
        LogError()(OT_PRETTY_CLASS())("failed to index ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    if (false == header_.Internal().AddHeader(block.Header().clone())) {
        LogError()(OT_PRETTY_CLASS())("failed to process ")(print(chain_))(
            " header")
            .Flush();

        return false;
    }

    return true;
}

auto Base::AddPeer(const blockchain::p2p::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    return peer_.AddPeer(address);
}

auto Base::BlockOracle() const noexcept -> const node::BlockOracle&
{
    return block_;
}

auto Base::BroadcastTransaction(
    const bitcoin::block::Transaction& tx,
    const bool pushtx) const noexcept -> bool
{
    mempool_.Submit(tx.clone());

    if (pushtx) {
        to_dht_.SendDeferred(
            [&] {
                auto out = network::zeromq::Message{};
                const auto command =
                    factory::BlockchainSyncPushTransaction(chain_, tx);
                command.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
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

auto Base::DB() const noexcept -> database::Database&
{
    OT_ASSERT(database_p_);

    return *database_p_;
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
    const auto http = wallet_.Internal().FeeEstimate();
    const auto fallback = params::get(chain_).FallbackTxFeeRate();
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

auto Base::FilterOracle() const noexcept -> const node::FilterOracle&
{
    OT_ASSERT(filter_p_);

    return *filter_p_;
}

auto Base::GetBalance() const noexcept -> Balance
{
    OT_ASSERT(database_p_);

    return database_p_->GetBalance();
}

auto Base::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    OT_ASSERT(database_p_);

    return database_p_->GetBalance(owner);
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

auto Base::GetShared() const noexcept -> std::shared_ptr<const node::Manager>
{
    init_.get();

    return self_.lock()->lock();
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

auto Base::HeaderOracle() const noexcept -> const node::HeaderOracle&
{
    return header_;
}

auto Base::init() noexcept -> void
{
    trigger();
    reset_heartbeat();
}

auto Base::JobReady(const node::PeerManagerJobs type) const noexcept -> void
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
    to_dht_.SendDeferred(
        [this] {
            const auto tip = filters_.FilterTip(filters_.DefaultType());
            using Job = network::blockchain::DHTJob;
            auto msg = MakeWork(Job::job_processed);
            msg.AddFrame(tip.height_);
            msg.AddFrame(tip.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
}

auto Base::PeerManager() const noexcept -> const internal::PeerManager&
{
    OT_ASSERT(peer_p_);

    return *peer_p_;
}

auto Base::pipeline(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    init_.get();
    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body.at(0).as<ManagerJobs>();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            OT_FAIL;
        }
    }();

    switch (task) {
        case ManagerJobs::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case ManagerJobs::sync_reply:
        case ManagerJobs::sync_new_block: {
            process_sync_data(std::move(in));
        } break;
        case ManagerJobs::submit_block: {
            process_block(std::move(in));
        } break;
        case ManagerJobs::heartbeat: {
            // TODO upgrade all the oracles to no longer require this
            mempool_.Heartbeat();
            filters_.Internal().Heartbeat();
            peer_.Heartbeat();
            do_work();
            reset_heartbeat();
        } break;
        case ManagerJobs::send_to_address: {
            process_send_to_address(std::move(in));
        } break;
        case ManagerJobs::send_to_paymentcode: {
            process_send_to_payment_code(std::move(in));
        } break;
        case ManagerJobs::start_wallet: {
            to_wallet_.SendDeferred(
                MakeWork(wallet::WalletJobs::start_wallet), __FILE__, __LINE__);
        } break;
        case ManagerJobs::filter_update: {
            process_filter_update(std::move(in));
        } break;
        case ManagerJobs::state_machine: {
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

    header_.Internal().SubmitBlock(body.at(1).Bytes());
    to_block_cache_.SendDeferred(
        [&] {
            auto out = MakeWork(blockoracle::CacheJob::process_block);
            out.AddFrame(std::move(body.at(1)));

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Base::process_filter_update(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto height = body.at(2).as<block::Height>();
    const auto target = header_.Target();

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

    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::BlockchainSyncProgress);
            work.AddFrame(chain_);
            work.AddFrame(height);
            work.AddFrame(target);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Base::process_send_to_address(network::zeromq::Message&& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto sender = api_.Factory().NymIDFromHash(body.at(1).Bytes());
    const auto address = UnallocatedCString{body.at(2).Bytes()};
    const auto amount = factory::Amount(body.at(3));
    const auto memo = UnallocatedCString{body.at(4).Bytes()};
    const auto promise = body.at(5).as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto pNym = api_.Wallet().Nym(sender);

        if (!pNym) {
            const auto error = UnallocatedCString{"Invalid sender "} +
                               sender.asBase58(api_.Crypto());
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

        auto id = api_.Factory().IdentifierFromRandom();
        auto proposal = proto::BlockchainTransactionProposal{};
        proposal.set_version(proposal_version_);
        proposal.set_id(id.asBase58(api_.Crypto()));
        proposal.set_initiator(sender.data(), sender.size());
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
                output.set_pubkeyhash(UnallocatedCString{data.Bytes()});
            } break;
            case Style::P2WSH: {
                output.set_segwit(true);
                [[fallthrough]];
            }
            case Style::P2SH: {
                output.set_scripthash(UnallocatedCString{data.Bytes()});
            } break;
            case Style::Unknown:
            case Style::P2TR:
            default: {
                rc = SendResult::UnsupportedAddressFormat;

                throw std::runtime_error{"Unsupported address type"};
            }
        }

        wallet_.Internal().ConstructTransaction(
            proposal, send_promises_.finish(promise));
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

    const auto nymID = api_.Factory().NymIDFromHash(body.at(1).Bytes());
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
                nymID.asBase58(api_.Crypto()) + ')'};
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
            out.set_id(
                api_.Factory().IdentifierFromRandom().asBase58(api_.Crypto()));
            out.set_initiator(nymID.data(), nymID.size());
            out.set_expires(
                Clock::to_time_t(Clock::now() + std::chrono::hours(1)));
            out.set_memo(memo);
            auto& txout = *out.add_output();
            txout.set_version(output_version_);
            amount.Serialize(writer(txout.mutable_amount()));
            txout.set_index(index.value());
            txout.set_paymentcodechannel(account.ID().asBase58(api_.Crypto()));
            const auto pubkey = api_.Factory().DataFromBytes(key.PublicKey());
            LogVerbose()(OT_PRETTY_CLASS())(" using derived public key ")
                .asHex(pubkey)(" at index ")(index.value())(
                    " for outgoing transaction")
                .Flush();
            txout.set_pubkey(UnallocatedCString{pubkey.Bytes()});
            txout.set_contact(UnallocatedCString{contact.Bytes()});

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

        wallet_.Internal().ConstructTransaction(
            proposal, send_promises_.finish(promise));
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
    auto prior = block::Hash{};
    auto hashes = Vector<block::Hash>{};
    const auto accepted =
        header_.Internal().ProcessSyncData(prior, hashes, data);

    if (0_uz < accepted) {
        const auto& blocks = data.Blocks();

        LogVerbose()("Accepted ")(accepted)(" of ")(blocks.size())(" ")(
            print(chain_))(" headers")
            .Flush();
        filters_.Internal().ProcessSyncData(prior, hashes, data);
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

auto Base::Profile() const noexcept -> BlockchainProfile
{
    return config_.profile_;
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
            pipeline_.Push(MakeWork(ManagerJobs::heartbeat));
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
    auto work = MakeWork(ManagerJobs::send_to_address);
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
    auto work = MakeWork(ManagerJobs::send_to_paymentcode);
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
        self_.lock()->reset();
        pipeline_.Close();
        shutdown_sender_.Activate();
        peer_.Shutdown();
        filters_.Internal().Shutdown();
        shutdown_sender_.Close();
        promise.set_value();
    }
}

auto Base::shutdown_timers() noexcept -> void { heartbeat_.Cancel(); }

auto Base::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto Base::Start(std::shared_ptr<const node::Manager> me) noexcept -> void
{
    auto ptr = [&] {
        auto handle = self_.lock();
        auto& self = *handle;
        self = std::move(me);
        auto out = self.lock();

        OT_ASSERT(out);

        return out;
    }();
    *(self_.lock()) = ptr;
    auto api = api_.Internal().GetShared();
    opentxs::network::blockchain::OTDHT{api, ptr}.Init();
    block_.Start(api, ptr);
    filters_.Internal().Init(api, ptr);
    init_promise_.set_value();
    header_.Start(api, ptr);
    filters_.Internal().Start();
    peer_.Start();
    wallet_.Internal().Init(api, ptr);
}

auto Base::StartWallet() noexcept -> void
{
    if (false == config_.disable_wallet_) {
        pipeline_.Push(MakeWork(ManagerJobs::start_wallet));
    }
}

auto Base::state_machine() noexcept -> bool { return false; }

auto Base::Wallet() const noexcept -> const node::Wallet& { return wallet_; }

Base::~Base()
{
    Shutdown().get();
    shutdown_timers();
}
}  // namespace opentxs::blockchain::node::implementation
