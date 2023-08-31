// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/manager/Actor.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <BlockchainTransactionProposedNotification.pb.h>
#include <BlockchainTransactionProposedOutput.pb.h>
#include <BlockchainTransactionProposedSweep.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <HDPath.pb.h>
#include <Identifier.pb.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <future>
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
#include "blockchain/node/manager/Shared.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
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
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "matterfi/PaymentCode.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
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
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
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
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/Data.hpp"
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
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

auto print(ManagerJobs in) noexcept -> std::string_view
{
    using enum ManagerJobs;
    static constexpr auto map =
        frozen::make_unordered_map<ManagerJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {sync_reply, "sync_reply"sv},
            {sync_new_block, "sync_new_block"sv},
            {heartbeat, "heartbeat"sv},
            {send_to_address, "send_to_address"sv},
            {send_to_paymentcode, "send_to_paymentcode"sv},
            {sweep, "sweep"sv},
            {start_wallet, "start_wallet"sv},
            {init, "init"sv},
            {filter_update, "filter_update"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid node::ManagerJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::manager
{
using enum opentxs::network::zeromq::socket::Direction;
using opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<node::Manager> self,
    std::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : ManagerActor(
          *api,
          LogTrace(),
          [&] {
              auto out = CString{alloc};
              out.append(print(shared->Chain()));
              out.append(" node manager"sv);

              return out;
          }(),
          0ms,
          std::move(batch),
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {shared->Endpoints().shutdown_publish_, Connect},
              {shared->Endpoints().new_filter_publish_, Connect},
          },
          {
              {shared->Endpoints().manager_pull_, Bind},
          },
          {},
          {
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().peer_manager_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().wallet_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {shared->Endpoints().otdht_pull_, Connect},
               }},
              {Push,
               Policy::Internal,
               {
                   {api->Endpoints().Internal().BlockchainMessageRouter(),
                    Connect},
               }},
          })
    , api_p_(std::move(api))
    , self_p_(std::move(self))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , self_(*self_p_)
    , shared_(*shared_p_)
    , to_peer_manager_(pipeline_.Internal().ExtraSocket(0))
    , to_wallet_(pipeline_.Internal().ExtraSocket(1))
    , to_dht_(pipeline_.Internal().ExtraSocket(2))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(3))
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
{
    log_(shared_.GetConfig().Print(get_allocator())).Flush();
}

auto Actor::create_or_load_subaccount(
    const identifier::Nym& senderNym,
    const PaymentCode& senderPC,
    const proto::HDPath& senderPath,
    const PaymentCode& recipient,
    const PasswordPrompt& reason,
    Set<PaymentCode>& notify) const noexcept -> const crypto::PaymentCode&
{
    const auto& account =
        api_.Crypto().Blockchain().Internal().PaymentCodeSubaccount(
            senderNym,
            senderPC,
            recipient,
            senderPath,
            shared_.Chain(),
            reason);

    if (0_uz == account.OutgoingNotificationCount()) {
        notify.emplace(recipient);
    }

    return account;
}

auto Actor::do_shutdown() noexcept -> void
{
    shared_.Shutdown();
    heartbeat_.Cancel();
    shared_p_.reset();
    self_p_.reset();
    api_p_.reset();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (self_.Internal().ShuttingDown())) {

        return true;
    }

    reset_heartbeat();

    return false;
}

auto Actor::extract_notifications(
    const std::span<const network::zeromq::Frame> in,
    const identifier::Nym& senderNym,
    const PaymentCode& senderPC,
    const proto::HDPath& senderPath,
    const PasswordPrompt& reason,
    SendResult& rc,
    alloc::Strategy alloc) const noexcept(false) -> Set<PaymentCode>
{
    auto out = Set<PaymentCode>{alloc.result_};

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

auto Actor::get_sender(const identifier::Nym& nymID, SendResult& rc) const
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
    auto out = std::make_pair(nym.PaymentCodePublic(), proto::HDPath{});
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

auto Actor::Init(boost::shared_ptr<Actor> me) noexcept -> void
{
    signal_startup(me);
}

auto Actor::notify_sync_client() const noexcept -> void
{
    to_dht_.SendDeferred(
        [this] {
            const auto& filters = shared_.FilterOracle();
            const auto tip = filters.FilterTip(filters.DefaultType());
            using Job = network::blockchain::DHTJob;
            auto msg = MakeWork(Job::job_processed);
            msg.AddFrame(tip.height_);
            msg.AddFrame(tip.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using enum ManagerJobs;

    switch (work) {
        case sync_reply:
        case sync_new_block: {
            process_sync_data(std::move(msg), monotonic);
        } break;
        case heartbeat: {
            process_heartbeat(std::move(msg), monotonic);
        } break;
        case send_to_address: {
            process_send_to_address(std::move(msg), monotonic);
        } break;
        case send_to_paymentcode: {
            process_send_to_payment_code(std::move(msg), monotonic);
        } break;
        case sweep: {
            process_sweep(std::move(msg), monotonic);
        } break;
        case start_wallet: {
            process_start_wallet(std::move(msg), monotonic);
        } break;
        case filter_update: {
            process_filter_update(std::move(msg), monotonic);
        } break;
        case shutdown:
        case init:
        case statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Actor::process_filter_update(Message&& in, allocator_type) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(2 < body.size());

    const auto height = body[2].as<block::Height>();
    const auto target = shared_.HeaderOracle().Internal().Target();

    {
        const auto progress =
            (0 == target) ? double{0}
                          : ((double(height) / double(target)) * double{100});
        auto display = std::stringstream{};
        display << std::setprecision(3) << progress << "%";

        if (false == shared_.GetConfig().disable_wallet_) {
            log_(print(shared_.Chain()))(" chain sync progress: ")(
                height)(" of ")(target)(" (")(display.str())(")")
                .Flush();
        }
    }

    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::BlockchainSyncProgress, true);
            work.AddFrame(shared_.Chain());
            work.AddFrame(height);
            work.AddFrame(target);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::process_heartbeat(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    // TODO upgrade all the oracles to no longer require this
    shared_.Mempool().Heartbeat();
    shared_.FilterOracle().Internal().Heartbeat();
    reset_heartbeat();
}

auto Actor::process_send_to_address(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
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

            if ((false == chains.contains(shared_.Chain())) || (!supported)) {
                rc = SendResult::AddressNotValidforChain;

                throw std::runtime_error{
                    "Address "s.append(address)
                        .append(" not valid for "sv)
                        .append(blockchain::print(shared_.Chain()))};
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
                    body.subspan(6),
                    nymID,
                    sender,
                    path,
                    reason,
                    rc,
                    {monotonic, monotonic});
                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    serialize_notification(stuff.first, r, stuff.second, out);
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
        shared_.Wallet().Internal().ConstructTransaction(
            proposal, shared_.Finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        shared_.Finish(promise).set_value({rc, block::TransactionHash{}});
    }
}

auto Actor::process_send_to_payment_code(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto body = in.Payload();

    OT_ASSERT(5_uz < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body[1].Bytes());
    const auto recipient =
        api_.Factory().PaymentCodeFromBase58(body[2].Bytes());
    const auto contact =
        api_.Crypto().Blockchain().Internal().Contacts().PaymentCodeToContact(
            recipient, shared_.Chain());
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
            body.subspan(6),
            nymID,
            sender,
            path,
            reason,
            rc,
            {monotonic, monotonic});
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
            log_(OT_PRETTY_CLASS())(" using derived public key ")
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

                matterfi::paymentcode_extra_notifications(account, notify);
                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    serialize_notification(fuckllvm.first, r, p, out);
                };
                std::for_each(notify.begin(), notify.end(), serialize);

                return out;
            }(index, path, pubkey);
        shared_.Wallet().Internal().ConstructTransaction(
            proposal, shared_.Finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        shared_.Finish(promise).set_value({rc, block::TransactionHash{}});
    }
}

auto Actor::process_start_wallet(Message&& in, allocator_type) noexcept -> void
{
    to_wallet_.SendDeferred(
        MakeWork(wallet::WalletJobs::start_wallet), __FILE__, __LINE__);
}

auto Actor::process_sweep(Message&& in, allocator_type monotonic) noexcept
    -> void
{
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

                if ((false == chains.contains(shared_.Chain())) ||
                    (!supported)) {
                    rc = SendResult::AddressNotValidforChain;

                    throw std::runtime_error{
                        "Address "s.append(destination)
                            .append(" not valid for "sv)
                            .append(blockchain::print(shared_.Chain()))};
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
                    sKey.set_chain(translate(
                        UnitToClaim(blockchain_to_unit(shared_.Chain()))));
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
                    body.subspan(6),
                    nymID,
                    sender,
                    path,
                    reason,
                    rc,
                    {monotonic, monotonic});
                // TODO add preemptive notifications
                const auto serialize = [&](const auto& r) {
                    serialize_notification(stuff.first, r, stuff.second, out);
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
        shared_.Wallet().Internal().ConstructTransaction(
            proposal, shared_.Finish(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        shared_.Finish(promise).set_value({rc, block::TransactionHash{}});
    }
}

auto Actor::process_sync_data(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto start = Clock::now();
    const auto sync = api_.Factory().BlockchainSyncMessage(in);
    const auto& data = sync->asData();
    auto prior = block::Hash{};
    auto hashes = Vector<block::Hash>{};
    const auto accepted =
        shared_.HeaderOracle().Internal().ProcessSyncData(prior, hashes, data);

    if (0_uz < accepted) {
        const auto& blocks = data.Blocks();

        log_("Accepted ")(accepted)(" of ")(blocks.size())(" ")(
            print(shared_.Chain()))(" headers")
            .Flush();
        shared_.FilterOracle().Internal().ProcessSyncData(
            prior, hashes, data, monotonic);
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                Clock::now() - start);
        log_("Processed ")(blocks.size())(" ")(print(shared_.Chain()))(
            " sync packets in ")(elapsed)
            .Flush();
    } else {
        log_("Invalid ")(print(shared_.Chain()))(" sync data").Flush();
    }

    notify_sync_client();
}

auto Actor::reset_heartbeat() noexcept -> void
{
    heartbeat_.SetRelative(heartbeat_interval_);
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

auto Actor::serialize_notification(
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

auto Actor::work(allocator_type) noexcept -> bool { return false; }

Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::manager
