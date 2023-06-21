// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Manager.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <BlockchainTransactionProposal.pb.h>
#include <BlockchainTransactionProposedNotification.pb.h>
#include <BlockchainTransactionProposedOutput.pb.h>
#include <BlockchainTransactionProposedSweep.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <HDPath.pb.h>
#include <Identifier.pb.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iosfwd>
#include <optional>
#include <ratio>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Factory.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
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
#include "opentxs/blockchain/bitcoin/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::implementation
{
using namespace std::literals;
using enum opentxs::network::zeromq::socket::Direction;
using opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

constexpr auto proposal_version_ = VersionNumber{1};
constexpr auto notification_version_ = VersionNumber{1};
constexpr auto output_version_ = VersionNumber{1};
constexpr auto sweep_version_ = VersionNumber{1};
constexpr auto sweep_key_version_ = VersionNumber{1};

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
              {endpoints.manager_pull_, Bind},
          },
          {},
          {
              {Push,
               Policy::Internal,
               {
                   {endpoints.peer_manager_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {endpoints.wallet_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {endpoints.otdht_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {api.Endpoints()
                        .Internal()
                        .Internal()
                        .BlockchainMessageRouter(),
                    Connect},
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
    , command_line_peers_(seednode)
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
    , database_(*database_p_)
    , filters_(*filter_p_)
    , wallet_()
    , to_peer_manager_(pipeline_.Internal().ExtraSocket(0))
    , to_wallet_(pipeline_.Internal().ExtraSocket(1))
    , to_dht_(pipeline_.Internal().ExtraSocket(2))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(3))
    , send_promises_()
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
    , init_promise_()
    , init_(init_promise_.get_future())
    , self_()
{
    OT_ASSERT(database_p_);
    OT_ASSERT(filter_p_);

    header_.Internal().Init();
    init_executor({UnallocatedCString{endpoints_.new_filter_publish_}});
    LogVerbose()(config_.Print()).Flush();  // TODO allocator
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

auto Base::AddBlock(const block::Block& block) const noexcept -> bool
{
    if (false == block.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("invalid ")(print(chain_))(" block")
            .Flush();

        return false;
    }

    const auto& id = block.ID();

    if (false == block_.SubmitBlock(block, {})) {  // TODO monotonic allocator
        LogError()(OT_PRETTY_CLASS())("failed to save ")(print(chain_))(
            " block ")
            .asHex(id)
            .Flush();

        return false;
    }

    // TODO monotonic allocator
    if (false == filters_.Internal().ProcessBlock(block, {})) {
        LogError()(OT_PRETTY_CLASS())("failed to index ")(print(chain_))(
            " block")
            .Flush();

        return false;
    }

    if (false == header_.Internal().AddHeader(block.Header())) {
        LogError()(OT_PRETTY_CLASS())("failed to process ")(print(chain_))(
            " header")
            .Flush();

        return false;
    }

    return true;
}

auto Base::AddPeer(const network::blockchain::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        using enum PeerManagerJobs;
        auto work = MakeWork(addpeer);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return to_peer_manager_.SendDeferred(
            std::move(work), __FILE__, __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Base::BlockOracle() const noexcept -> const node::BlockOracle&
{
    return block_;
}

auto Base::BroadcastTransaction(const block::Transaction& tx, const bool pushtx)
    const noexcept -> bool
{
    mempool_.Submit(tx);

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

    using enum PeerManagerJobs;
    auto message = MakeWork(broadcasttx);

    if (false == tx.Internal()
                     .asBitcoin()
                     .Serialize(message.AppendBytes())
                     .has_value()) {

        return false;
    }

    return to_peer_manager_.SendDeferred(
        std::move(message), __FILE__, __LINE__);
}

auto Base::create_or_load_subaccount(
    const identifier::Nym& senderNym,
    const PaymentCode& senderPC,
    const proto::HDPath& senderPath,
    const PaymentCode& recipient,
    const PasswordPrompt& reason,
    Set<PaymentCode>& notify) const noexcept -> const crypto::PaymentCode&
{
    const auto& account =
        api_.Crypto().Blockchain().Internal().PaymentCodeSubaccount(
            senderNym, senderPC, recipient, senderPath, chain_, reason);

    if (false == account.IsNotified()) { notify.emplace(recipient); }

    return account;
}

auto Base::DB() const noexcept -> database::Database&
{
    OT_ASSERT(database_p_);

    return *database_p_;
}

auto Base::extract_notifications(
    const std::span<const network::zeromq::Frame> in,
    const identifier::Nym& senderNym,
    const PaymentCode& senderPC,
    const proto::HDPath& senderPath,
    const PasswordPrompt& reason,
    SendResult& rc) const noexcept(false) -> Set<PaymentCode>
{
    // TODO allocator
    auto out = Set<PaymentCode>{};

    for (const auto& frame : in) {
        const auto recipient =
            api_.Factory().PaymentCodeFromBase58(frame.Bytes());

        if (const auto version = recipient.Version(); 3 > version) {
            rc = SendResult::UnsupportedRecipientPaymentCode;

            throw std::runtime_error{
                "recipient "s + recipient.asBase58() +
                " has unsupported version " + std::to_string(version)};
        }

        create_or_load_subaccount(
            senderNym, senderPC, senderPath, recipient, reason, out);
    }

    return out;
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

auto Base::get_sender(const identifier::Nym& nymID, SendResult& rc) const
    noexcept(false) -> std::pair<opentxs::PaymentCode, proto::HDPath>
{
    const auto pNym = api_.Wallet().Nym(nymID);

    if (!pNym) {
        rc = SendResult::InvalidSenderNym;

        throw std::runtime_error{
            "Unable to load sender nym ("s + nymID.asBase58(api_.Crypto()) +
            ')'};
    }

    const auto& nym = *pNym;
    auto out = std::make_pair(
        api_.Factory().PaymentCodeFromBase58(nym.PaymentCode()),
        proto::HDPath{});
    auto& [sender, path] = out;

    if (0 == sender.Version()) {
        rc = SendResult::SenderMissingPaymentCode;

        throw std::runtime_error{"Invalid sender payment code"};
    }

    if (false == nym.Internal().PaymentCodePath(path)) {
        rc = SendResult::HDDerivationFailure;

        throw std::runtime_error{"Failed to obtain payment code HD path"};
    }

    return out;
}

auto Base::GetShared() const noexcept -> std::shared_ptr<const node::Manager>
{
    init_.get();

    return self_.lock()->lock();
}

auto Base::GetTransactions() const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return database_.GetTransactions();
}

auto Base::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return database_.GetTransactions(account);
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

auto Base::Listen(const network::blockchain::Address& address) const noexcept
    -> bool
{
    if (false == running_.load()) { return false; }

    try {
        const auto proto = [&] {
            auto out = proto::BlockchainPeerAddress{};

            if (false == address.Internal().Serialize(out)) {
                throw std::runtime_error{
                    "failed to serialize address to protobuf"};
            }

            return out;
        }();
        using enum PeerManagerJobs;
        auto work = MakeWork(addlistener);

        if (false == proto::write(proto, work.AppendBytes())) {
            throw std::runtime_error{"failed to serialize protobuf to bytes"};
        }

        return to_peer_manager_.SendDeferred(
            std::move(work), __FILE__, __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
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

auto Base::pipeline(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    init_.get();
    const auto body = in.Payload();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body[0].as<ManagerJobs>();
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
        case ManagerJobs::heartbeat: {
            // TODO upgrade all the oracles to no longer require this
            mempool_.Heartbeat();
            filters_.Internal().Heartbeat();
            do_work();
            reset_heartbeat();
        } break;
        case ManagerJobs::send_to_address: {
            process_send_to_address(std::move(in));
        } break;
        case ManagerJobs::send_to_paymentcode: {
            process_send_to_payment_code(std::move(in));
        } break;
        case ManagerJobs::sweep: {
            process_sweep(std::move(in));
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

auto Base::process_filter_update(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    OT_ASSERT(2 < body.size());

    const auto height = body[2].as<block::Height>();
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
                WorkType::BlockchainSyncProgress, true);
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

    const auto body = in.Payload();

    OT_ASSERT(5_uz < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body[1].Bytes());
    const auto address = UnallocatedCString{body[2].Bytes()};
    const auto amount = factory::Amount(body[3]);
    const auto memo = body[4].Bytes();
    const auto promise = body[5].as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto proposal = [&] {
            const auto [data, style, chains, supported] =
                api_.Crypto().Blockchain().DecodeAddress(address);

            if ((false == chains.contains(chain_)) || (!supported)) {
                rc = SendResult::AddressNotValidforChain;

                throw std::runtime_error{
                    "Address "s.append(address)
                        .append(" not valid for "sv)
                        .append(blockchain::print(chain_))};
            }

            const auto id = api_.Factory().IdentifierFromRandom();
            auto out = proto::BlockchainTransactionProposal{};
            out.set_version(proposal_version_);
            out.set_id(id.asBase58(api_.Crypto()));
            out.set_initiator(nymID.data(), nymID.size());
            out.set_expires(Clock::to_time_t(Clock::now() + 1h));
            out.set_memo(UnallocatedCString{memo});

            {
                auto& txout = *out.add_output();
                txout.set_version(output_version_);
                amount.Serialize(writer(txout.mutable_amount()));
                using enum blockchain::crypto::AddressStyle;

                switch (style) {
                    case P2WPKH: {
                        txout.set_segwit(true);
                        [[fallthrough]];
                    }
                    case P2PKH: {
                        txout.set_pubkeyhash(UnallocatedCString{data.Bytes()});
                    } break;
                    case P2WSH: {
                        txout.set_segwit(true);
                        [[fallthrough]];
                    }
                    case P2SH: {
                        txout.set_scripthash(UnallocatedCString{data.Bytes()});
                    } break;
                    case Unknown:
                    case P2TR:
                    default: {
                        rc = SendResult::UnsupportedAddressFormat;

                        throw std::runtime_error{"Unsupported address type"};
                    }
                }
            }

            if (body.size() > 6_uz) {
                const auto reason = api_.Factory().PasswordPrompt(
                    "constructing payment code notifications"sv);
                // TODO c++20
                const auto stuff = get_sender(nymID, rc);
                const auto [sender, path] = stuff;
                const auto notify = extract_notifications(
                    body.subspan(6), nymID, sender, path, reason, rc);
                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    Base::serialize_notification(
                        stuff.first, r, stuff.second, out);
                };
                std::for_each(notify.begin(), notify.end(), serialize);
            } else {
                const auto pNym = api_.Wallet().Nym(nymID);

                if (!pNym) {
                    const auto error =
                        "Invalid sender "s + nymID.asBase58(api_.Crypto());
                    rc = SendResult::InvalidSenderNym;

                    throw std::runtime_error{error};
                }
            }

            return out;
        }();
        wallet_.Internal().ConstructTransaction(
            proposal, send_promises_.finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        send_promises_.finish(promise).set_value(
            {rc, block::TransactionHash{}});
    }
}

auto Base::process_send_to_payment_code(network::zeromq::Message&& in) noexcept
    -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    OT_ASSERT(5_uz < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body[1].Bytes());
    const auto recipient =
        api_.Factory().PaymentCodeFromBase58(body[2].Bytes());
    const auto contact =
        api_.Crypto().Blockchain().Internal().Contacts().PaymentCodeToContact(
            recipient, chain_);
    const auto amount = factory::Amount(body[3]);
    const auto memo = body[4].Bytes();
    const auto promise = body[5].as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto fuckllvm = get_sender(nymID, rc);
        const auto& [sender, path] = fuckllvm;
        const auto reason = api_.Factory().PasswordPrompt(
            "Sending a transaction to "s + recipient.asBase58());
        auto notify = extract_notifications(
            body.subspan(6), nymID, sender, path, reason, rc);
        const auto& account = create_or_load_subaccount(
            nymID, sender, path, recipient, reason, notify);
        const auto [index, pubkey] = [&] {
            auto out = std::pair<Bip32Index, ByteArray>{};
            constexpr auto subchain{crypto::Subchain::Outgoing};
            const auto i = account.Reserve(subchain, reason);

            if (i.has_value()) {
                out.first = *i;
            } else {
                rc = SendResult::HDDerivationFailure;

                throw std::runtime_error{"Failed to allocate next key"};
            }

            const auto& key = [&]() -> const auto& {
                const auto& element =
                    account.BalanceElement(subchain, i.value());
                const auto& k = element.Key();

                if (false == k.IsValid()) {
                    rc = SendResult::HDDerivationFailure;

                    throw std::runtime_error{"Failed to instantiate key"};
                }

                return k;
            }();
            out.second.Assign(key.PublicKey());
            LogVerbose()(OT_PRETTY_CLASS())(" using derived public key ")
                .asHex(out.second)(" at index ")(out.first)(
                    " for outgoing transaction")
                .Flush();

            return out;
        }();
        // TODO c++20
        const auto proposal =
            [&](const auto& i, const auto& p, const auto& pk) {
                const auto id = api_.Factory().IdentifierFromRandom();
                auto out = proto::BlockchainTransactionProposal{};
                out.set_version(proposal_version_);
                out.set_id(id.asBase58(api_.Crypto()));
                out.set_initiator(nymID.data(), nymID.size());
                out.set_expires(Clock::to_time_t(Clock::now() + 1h));
                out.set_memo(UnallocatedCString{memo});

                {
                    auto& txout = *out.add_output();
                    txout.set_version(output_version_);
                    amount.Serialize(writer(txout.mutable_amount()));
                    txout.set_index(i);
                    txout.set_paymentcodechannel(
                        account.ID().asBase58(api_.Crypto()));
                    txout.set_pubkey(UnallocatedCString{pk.Bytes()});
                    txout.set_contact(UnallocatedCString{contact.Bytes()});
                }

                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    Base::serialize_notification(fuckllvm.first, r, p, out);
                };
                std::for_each(notify.begin(), notify.end(), serialize);

                return out;
            }(index, path, pubkey);
        wallet_.Internal().ConstructTransaction(
            proposal, send_promises_.finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        send_promises_.finish(promise).set_value(
            {rc, block::TransactionHash{}});
    }
}

auto Base::process_sweep(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Payload();

    OT_ASSERT(5_uz < body.size());

    const auto nymID = api_.Factory().Internal().NymID(
        proto::Factory<proto::Identifier>(body[1]));
    const auto subaccount = [&] {
        const auto& frame = body[2];

        if (0_uz < frame.size()) {

            return api_.Factory().Internal().AccountID(
                proto::Factory<proto::Identifier>(frame));
        } else {

            return identifier::Account{};
        }
    }();
    const auto key = [&] {
        const auto& frame = body[3];
        auto out = std::optional<crypto::Key>{};

        if (0_uz < frame.size()) { out.emplace(deserialize(frame.Bytes())); }

        return out;
    }();
    const auto destination = body[4].Bytes();
    const auto promise = body[5].as<int>();
    auto rc = SendResult::UnspecifiedError;

    try {
        const auto proposal = [&] {
            const auto id = api_.Factory().IdentifierFromRandom();
            auto out = proto::BlockchainTransactionProposal{};
            out.set_version(proposal_version_);
            out.set_id(id.asBase58(api_.Crypto()));
            out.set_initiator(nymID.data(), nymID.size());
            out.set_expires(Clock::to_time_t(Clock::now() + 1h));
            out.set_memo([&] {
                auto memo = std::stringstream{"sweep"};

                if (valid(destination)) { memo << " to " << destination; }

                return memo.str();
            }());
            const auto hasNotifications = body.size() > 6_uz;

            if (valid(destination)) {
                if (hasNotifications) {
                    rc = SendResult::InvalidSweep;

                    throw std::runtime_error{
                        "sweep to address may not perform notifications"};
                }

                const auto [data, style, chains, supported] =
                    api_.Crypto().Blockchain().DecodeAddress(destination);

                if ((false == chains.contains(chain_)) || (!supported)) {
                    rc = SendResult::AddressNotValidforChain;

                    throw std::runtime_error{
                        "Address "s.append(destination)
                            .append(" not valid for "sv)
                            .append(blockchain::print(chain_))};
                }

                auto& txout = *out.add_output();
                txout.set_version(output_version_);
                using enum blockchain::crypto::AddressStyle;

                switch (style) {
                    case P2WPKH: {
                        txout.set_segwit(true);
                        [[fallthrough]];
                    }
                    case P2PKH: {
                        txout.set_pubkeyhash(UnallocatedCString{data.Bytes()});
                    } break;
                    case P2WSH: {
                        txout.set_segwit(true);
                        [[fallthrough]];
                    }
                    case P2SH: {
                        txout.set_scripthash(UnallocatedCString{data.Bytes()});
                    } break;
                    case Unknown:
                    case P2TR:
                    default: {
                        rc = SendResult::UnsupportedAddressFormat;

                        throw std::runtime_error{"Unsupported address type"};
                    }
                }
            }

            {
                auto& sweep = *out.mutable_sweep();
                sweep.set_version(sweep_version_);

                if (key.has_value()) {
                    const auto& [_, subchain, index] = *key;
                    auto& sKey = *sweep.mutable_key();
                    sKey.set_version(sweep_key_version_);
                    sKey.set_chain(
                        translate(UnitToClaim(BlockchainToUnit(chain_))));
                    sKey.set_nym(nymID.asBase58(api_.Crypto()));
                    sKey.set_subaccount(subaccount.asBase58(api_.Crypto()));
                    sKey.set_subchain(static_cast<std::uint32_t>(subchain));
                    sKey.set_index(index);
                } else if (false == subaccount.empty()) {
                    subaccount.Internal().Serialize(
                        *sweep.mutable_subaccount());
                }
            }

            if (hasNotifications) {
                const auto reason = api_.Factory().PasswordPrompt(
                    "constructing payment code notifications"sv);
                // TODO c++20
                const auto stuff = get_sender(nymID, rc);
                const auto [sender, path] = stuff;
                const auto notify = extract_notifications(
                    body.subspan(6), nymID, sender, path, reason, rc);
                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    Base::serialize_notification(
                        stuff.first, r, stuff.second, out);
                };
                std::for_each(notify.begin(), notify.end(), serialize);
            } else {
                const auto pNym = api_.Wallet().Nym(nymID);

                if (!pNym) {
                    rc = SendResult::InvalidSenderNym;

                    throw std::runtime_error{
                        "Invalid sender "s + nymID.asBase58(api_.Crypto())};
                }
            }

            return out;
        }();
        wallet_.Internal().ConstructTransaction(
            proposal, send_promises_.finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        send_promises_.finish(promise).set_value(
            {rc, block::TransactionHash{}});
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
        // TODO monotonic allocator
        filters_.Internal().ProcessSyncData(prior, hashes, data, {});
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
    heartbeat_.Wait([this](const auto& ec) {
        if (ec) {
            if (unexpected_asio_error(ec)) {
                LogError()(OT_PRETTY_CLASS())("received asio error (")(
                    ec.value())(") :")(ec)
                    .Flush();
            }
        } else {
            pipeline_.Push(MakeWork(ManagerJobs::heartbeat));
        }
    });
}

auto Base::SendToAddress(
    const opentxs::identifier::Nym& sender,
    std::string_view address,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    // TODO c++20
    pipeline_.Push([&](const auto& i) {
        auto work = MakeWork(ManagerJobs::send_to_address);
        work.AddFrame(sender);
        work.AddFrame(address.data(), address.size());
        amount.Serialize(work.AppendBytes());
        work.AddFrame(memo.data(), memo.size());
        work.AddFrame(i);
        serialize_notifications(notify, work);

        return work;
    }(index));

    return std::move(future);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    std::string_view recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const std::string_view> notify) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    // TODO c++20
    pipeline_.Push([&](const auto& i) {
        auto work = MakeWork(ManagerJobs::send_to_paymentcode);
        work.AddFrame(nymID);
        work.AddFrame(recipient.data(), recipient.size());
        amount.Serialize(work.AppendBytes());
        work.AddFrame(memo.data(), memo.size());
        work.AddFrame(i);
        serialize_notifications(notify, work);

        return work;
    }(index));

    return std::move(future);
}

auto Base::SendToPaymentCode(
    const opentxs::identifier::Nym& nymID,
    const PaymentCode& recipient,
    const Amount amount,
    std::string_view memo,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    // TODO c++20
    pipeline_.Push([&](const auto& i) {
        auto work = MakeWork(ManagerJobs::send_to_paymentcode);
        work.AddFrame(nymID);
        work.AddFrame(recipient.asBase58());
        amount.Serialize(work.AppendBytes());
        work.AddFrame(memo.data(), memo.size());
        work.AddFrame(i);
        serialize_notifications(notify, work);

        return work;
    }(index));

    return std::move(future);
}

auto Base::serialize_notification(
    const PaymentCode& sender,
    const PaymentCode& recipient,
    const proto::HDPath& senderPath,
    proto::BlockchainTransactionProposal& out) noexcept -> void
{
    auto& notif = *out.add_notification();
    notif.set_version(notification_version_);
    *notif.mutable_path() = senderPath;
    sender.Internal().Serialize(*notif.mutable_sender());
    recipient.Internal().Serialize(*notif.mutable_recipient());
}

auto Base::serialize_notifications(
    std::span<const std::string_view> in,
    network::zeromq::Message& out) noexcept -> void
{
    auto append_to_message = [&out](const auto& i) {
        out.AddFrame(i.data(), i.size());
    };
    std::for_each(in.begin(), in.end(), append_to_message);
}

auto Base::serialize_notifications(
    std::span<const PaymentCode> in,
    network::zeromq::Message& out) noexcept -> void
{
    auto append_to_message = [&out](const auto& i) {
        out.AddFrame(i.asBase58());
    };
    std::for_each(in.begin(), in.end(), append_to_message);
}

auto Base::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        init_.get();
        self_.lock()->reset();
        pipeline_.Close();
        shutdown_sender_.Activate();
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
    factory::BlockchainPeerManager(api, ptr, database_, command_line_peers_);
    wallet_.Internal().Init(api, ptr);
}

auto Base::StartWallet() noexcept -> void
{
    if (false == config_.disable_wallet_) {
        pipeline_.Push(MakeWork(ManagerJobs::start_wallet));
    }
}

auto Base::state_machine() noexcept -> bool { return false; }

auto Base::Sweep(
    const identifier::Nym& account,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    static const auto blankSubaccount = identifier::Account{};

    return sweep(account, blankSubaccount, std::nullopt, toAddress, notify);
}

auto Base::Sweep(
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    return sweep(account, subaccount, std::nullopt, toAddress, notify);
}

auto Base::Sweep(
    const crypto::Key& key,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    const auto& [subaccount, subchain, index] = key;
    const auto [_, nym] = api_.Crypto().Blockchain().LookupAccount(subaccount);

    return sweep(nym, subaccount, key, toAddress, notify);
}

auto Base::sweep(
    const identifier::Nym& account,
    const identifier::Account& subaccount,
    std::optional<crypto::Key> key,
    std::string_view toAddress,
    std::span<const PaymentCode> notify) const noexcept -> PendingOutgoing
{
    auto [index, future] = send_promises_.get();
    // TODO c++20
    pipeline_.Push([&](const auto& i) {
        auto work = MakeWork(ManagerJobs::sweep);
        account.Internal().Serialize(work);

        if (subaccount.empty()) {
            work.AddFrame();
        } else {
            subaccount.Internal().Serialize(work);
        }

        if (key.has_value()) {
            work.AddFrame(serialize(*key));
        } else {
            work.AddFrame();
        }

        work.AddFrame(toAddress.data(), toAddress.size());
        work.AddFrame(i);
        serialize_notifications(notify, work);

        return work;
    }(index));

    return std::move(future);
}

auto Base::Wallet() const noexcept -> const node::Wallet& { return wallet_; }

Base::~Base()
{
    Shutdown().get();
    shutdown_timers();
}
}  // namespace opentxs::blockchain::node::implementation
