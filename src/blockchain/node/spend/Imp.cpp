// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/spend/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <BlockchainTransactionProposal.pb.h>
#include <BlockchainTransactionProposedNotification.pb.h>
#include <BlockchainTransactionProposedOutput.pb.h>
#include <BlockchainTransactionProposedSweep.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <HDPath.pb.h>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/crypto/PaymentCode.hpp"
#include "internal/blockchain/node/SpendPolicy.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Time.hpp"
#include "matterfi/PaymentCode.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/blockchain/node/Funding.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

SpendPrivate::SpendPrivate(
    const api::session::Client& api,
    VersionNumber version,
    blockchain::Type chain,
    identifier::Generic id,
    identifier::Nym spender,
    Time expires,
    opentxs::PasswordPrompt reason) noexcept
    : api_(api)
    , version_(version)
    , chain_(chain)
    , id_(std::move(id))
    , spender_(std::move(spender))
    , nym_(api_.Wallet().Nym(spender_))
    , expires_(expires)
    , reason_(std::move(reason))
    , memo_()
    , address_recipients_()
    , pc_recipients_()
    , notifications_()
    , policy_(Funding::Default)
    , spend_unconfirmed_change_(true)
    , spend_unconfirmed_incoming_(params::get(chain).SpendUnconfirmed())
    , use_enhanced_notifications_(true)
    , payment_code_path_()
    , sender_payment_code_()
    , sweep_to_address_()
    , sweep_subaccount_()
    , sweep_key_()
    , transaction_()
{
    static constexpr auto prepare = [](auto& v) {
        v.reserve(max_outputs_);
        v.clear();
    };
    prepare(address_recipients_);
    prepare(pc_recipients_);
}

SpendPrivate::SpendPrivate(
    const api::session::Client& api,
    blockchain::Type chain,
    const identifier::Nym& spender,
    opentxs::PasswordPrompt reason) noexcept
    : SpendPrivate(
          api,
          proposal_version_,
          chain,
          api.Factory().IdentifierFromRandom(),
          spender,
          Clock::now() + 1h,
          std::move(reason))
{
}

SpendPrivate::SpendPrivate(
    const api::session::Client& api,
    blockchain::Type chain,
    const proto::BlockchainTransactionProposal& proto) noexcept(false)
    : SpendPrivate(
          api,
          proto.version(),
          chain,
          api.Factory().IdentifierFromBase58(proto.id()),
          api.Factory().NymIDFromHash(proto.initiator()),
          convert_time(proto.expires()),
          api.Factory().PasswordPrompt(proto.password_prompt()))
{
    memo_.assign(proto.memo());
    const auto& outputs = proto.output();
    std::ranges::for_each(
        outputs, [this](const auto& i) { deserialize_output(i); });
    const auto& notifications = proto.notification();
    std::ranges::for_each(
        notifications, [this](const auto& i) { deserialize_notification(i); });

    if (proto.has_finished()) { transaction_.emplace(proto.finished()); }

    if (proto.has_sweep()) { deserialize_sweep(proto.sweep()); }

    spend_unconfirmed_change_ = proto.spend_unconfirmed_change();
    spend_unconfirmed_incoming_ = proto.spend_unconfirmed_incoming();
}

auto SpendPrivate::account(const PaymentCode& recipient) const noexcept(false)
    -> const crypto::PaymentCode&
{
    if (nym().PaymentCodePublic() == recipient) {
        LogAbort()()("attempted to create loopback payment code account")
            .Abort();
    }

    const auto& out = api_.Crypto().Blockchain().LoadOrCreateSubaccount(
        spender_, recipient, chain_, reason_);

    if (false == out.IsValid()) {
        throw std::runtime_error{
            "failed to instantiate or load account for "s.append(
                recipient.asBase58())};
    }

    return out;
}

auto SpendPrivate::Add(const proto::BlockchainTransaction& tx) noexcept -> void
{
    transaction_.emplace(tx);
}

auto SpendPrivate::AddNotification(
    const block::TransactionHash& txid) const noexcept -> void
{
    const auto self = nym().PaymentCodePublic();

    for (const auto& recipient : notifications_) {
        try {
            if (self != recipient) {
                account(recipient).InternalPaymentCode().AddNotification(txid);
            }
        } catch (const std::exception& e) {
            LogError()()(
                "unable to record outgoing notification transaction for ")(
                recipient.asBase58())(": ")(e.what())
                .Flush();

            continue;
        }
    }
}

auto SpendPrivate::AddressRecipients() const noexcept
    -> std::span<const AddressRecipient>
{
    return address_recipients_;
}

auto SpendPrivate::add_address(
    std::string_view bytes,
    crypto::AddressStyle type,
    const Amount& amount) noexcept(false) -> void
{
    auto& data = address_recipients_;

    if (data.size() >= max_outputs_) {

        throw std::runtime_error{
            "maximum number of address recipients reached"};
    }

    data.emplace_back(identifier::Generic{}, amount, type, bytes);
}

auto SpendPrivate::add_payment_code(
    const PaymentCode& recipient,
    const Amount& amount,
    const identifier::Account& subaccount,
    Bip32Index index,
    ReadView pubkey) noexcept(false) -> void
{
    validate_payment_code(recipient);
    check_send_to_self(recipient);
    auto& data = pc_recipients_;

    if (data.size() >= max_outputs_) {

        throw std::runtime_error{
            "maximum number of payment code recipients reached"};
    }

    data.emplace_back(
        api_.Contacts().PaymentCodeToContact(recipient, chain_),
        amount,
        recipient,
        crypto::Key{subaccount, crypto::Subchain::Outgoing, index},
        pubkey);
}

auto SpendPrivate::Check() noexcept -> std::optional<SendResult>
{
    using enum SendResult;
    using enum node::Funding;

    if (false == check_sender()) { return InvalidSenderNym; }

    switch (policy_) {
        case Default: {

            return check_funding_default();
        }
        case SweepAccount:
        case SweepSubaccount:
        case SweepKey: {

            return check_funding_sweep();
        }
        default: {
            LogError()()("invalid funding policy: ")(print(policy_)).Flush();

            return SendResult::UnspecifiedError;
        }
    }
}

auto SpendPrivate::check_funding_default() const noexcept
    -> std::optional<SendResult>
{
    if (address_recipients_.empty() && pc_recipients_.empty()) {
        LogError()()("no recipients specified").Flush();

        return SendResult::MissingRecipients;
    } else {

        return std::nullopt;
    }
}

auto SpendPrivate::check_funding_sweep() noexcept -> std::optional<SendResult>
{
    try {
        if (sweep_to_address_.has_value()) { return std::nullopt; }

        if (false == notifications_.empty()) { return std::nullopt; }

        // NOTE it's fine even if neither a sweep to address nor any
        // notifications are specified since the transaction builder will
        // automatically add a change output in that case.

        return std::nullopt;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return SendResult::InvalidSweep;
    }
}

auto SpendPrivate::check_send_to_self(const PaymentCode& pc) const
    noexcept(false) -> void
{
    if (sender_payment_code() == pc) {
        throw std::runtime_error{"attempting to send to self via payment code "s
                                     .append(pc.asBase58())
                                     .append(" belonging to nym ")
                                     .append(spender_.asBase58(api_.Crypto()))};
    }
}

auto SpendPrivate::check_sender() const noexcept -> bool
{
    try {
        const auto& nym = this->nym();
        using enum identity::NymCapability;

        if (false == nym.HasCapability(SIGN_MESSAGE)) {
            throw std::runtime_error{
                "nym "s.append(spender_.asBase58(api_.Crypto()))
                    .append(" does not contain private keys")};
        }

        if ((!pc_recipients_.empty()) || (!notifications_.empty())) {
            if (false == sender_payment_code().Valid()) {
                throw std::runtime_error{
                    "nym "s.append(spender_.asBase58(api_.Crypto()))
                        .append(" does not contain a valid payment code")};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::deserialize_notification(
    const proto::BlockchainTransactionProposedNotification& in) noexcept(false)
    -> void
{
    const auto sender =
        api_.Factory().InternalSession().PaymentCode(in.sender());

    if (sender != sender_payment_code()) {
        throw std::runtime_error{"invalid sender payment code in notification"};
    }

    if (in.path() != path()) {
        throw std::runtime_error{"invalid hd path in notification"};
    }

    notifications_.emplace(
        api_.Factory().InternalSession().PaymentCode(in.recipient()));
}

auto SpendPrivate::deserialize_output(
    const proto::BlockchainTransactionProposedOutput& in) noexcept(false)
    -> void
{
    using enum crypto::AddressStyle;
    const auto amount = factory::Amount(in.amount());
    validate_amount(amount);
    const auto isAddress = in.has_pubkeyhash() || in.has_scripthash();

    if (isAddress) {
        const auto [type, bytes] = [&] {
            const auto isSegwit = in.segwit();

            if (in.has_pubkeyhash()) {

                return std::make_pair(
                    isSegwit ? P2WPKH : P2PKH, ByteArray{in.pubkeyhash()});
            } else {

                return std::make_pair(
                    isSegwit ? P2WSH : P2SH, ByteArray{in.scripthash()});
            }
        }();
        const auto address =
            api_.Crypto().Blockchain().EncodeAddress(type, chain_, bytes);

        if (address.empty()) { throw std::runtime_error{"invalid address"}; }

        add_address(bytes.Bytes(), type, amount);
    } else if (in.has_pubkey()) {
        auto subaccountID =
            api_.Factory().AccountIDFromBase58(in.paymentcodechannel());
        const auto& subaccount = api_.Crypto()
                                     .Blockchain()
                                     .Wallet(chain_)
                                     .Account(spender_)
                                     .GetPaymentCode()
                                     .at(subaccountID);
        add_payment_code(
            subaccount.Remote(),
            amount,
            subaccountID,
            static_cast<Bip32Index>(in.index()),
            in.pubkey());
    } else {
        throw std::runtime_error{"unsupported output type"};
    }
}

auto SpendPrivate::deserialize_sweep(
    const proto::BlockchainTransactionProposedSweep& in) noexcept(false) -> void
{
    using enum node::Funding;

    if (in.has_subaccount()) {
        policy_ = SweepSubaccount;
        sweep_subaccount_.emplace(
            api_.Factory().Internal().AccountID(in.subaccount()));
    } else if (in.has_key()) {
        policy_ = SweepKey;
        const auto& key = in.key();
        const auto chain =
            unit_to_blockchain(ClaimToUnit(translate(key.chain())));

        if (chain != chain_) {
            throw std::runtime_error{"notification for incorrect chain"};
        }

        const auto sender = api_.Factory().NymIDFromBase58(key.nym());

        if (sender != spender_) {
            throw std::runtime_error{"notification from incorrect spender"};
        }

        sweep_key_.emplace(
            api_.Factory().AccountIDFromBase58(key.subaccount()),
            static_cast<crypto::Subchain>(key.subchain()),
            static_cast<Bip32Index>(key.index()));
    } else {
        policy_ = SweepAccount;
    }
}

auto SpendPrivate::Finalize(const Log& log, alloc::Strategy alloc) noexcept(
    false) -> void
{
    if (use_enhanced_notifications_) {
        const auto before = notifications_.size();
        const auto check = [this](const auto& item) {
            const auto& [contact, amount, recipient, keyID, pubkey] = item;
            const auto& [accountID, subchain, index] = keyID;
            const auto& account = api_.Crypto()
                                      .Blockchain()
                                      .Wallet(chain_)
                                      .Account(spender_)
                                      .GetPaymentCode()
                                      .at(accountID);
            matterfi::paymentcode_extra_notifications(
                LogTrace(), account, notifications_);
        };
        std::ranges::for_each(pc_recipients_, check);
        const auto self = notifications_.size();
        matterfi::paymentcode_preemptive_notifications(
            log, api_, spender_, chain_, notifications_, alloc);
        log()("added ")(self - before)(" self notifications for ")(
            pc_recipients_.size())(" recipients")
            .Flush();
        log()("added ")(notifications_.size() - self)(
            " preemptive notifications")
            .Flush();
    } else {
        log()("skipping enhanced notifications").Flush();
    }

    const auto self = nym().PaymentCodePublic();
    const auto check = [&, this](const auto& recipient) {
        if (self != recipient) {
            const auto& account = this->account(recipient);
            log()("verifying payment code subaccount ")(
                account.ID(), api_.Crypto())
                .Flush();
        }
    };
    std::ranges::for_each(notifications_, check);
}

auto SpendPrivate::Funding() const noexcept -> node::Funding { return policy_; }

auto SpendPrivate::ID() const noexcept -> const identifier::Generic&
{
    return id_;
}

auto SpendPrivate::IsExpired() const noexcept -> bool
{
    return Clock::now() > expires_;
}

auto SpendPrivate::is_sweep() const noexcept -> bool
{
    using enum node::Funding;

    switch (policy_) {
        case SweepAccount:
        case SweepSubaccount:
        case SweepKey: {

            return true;
        }
        case Default:
        default: {

            return false;
        }
    }
}

auto SpendPrivate::Memo() const noexcept -> std::string_view { return memo_; }

auto SpendPrivate::Notifications() const noexcept
    -> std::span<const PaymentCode>
{
    // NOTE Some compilers using some versions of Boost can directly construct a
    // std::span from a boost::container::flat_set as long as it's backed by a
    // vector but in other configurations it doesn't work.
    if (notifications_.empty()) {

        return {};
    } else {

        return {std::addressof(*notifications_.begin()), notifications_.size()};
    }
}

auto SpendPrivate::Notify(const PaymentCode& recipient) noexcept -> bool
{
    return Notify({std::addressof(recipient), 1_uz});
}

auto SpendPrivate::Notify(std::span<const PaymentCode> recipients) noexcept
    -> bool
{
    try {
        auto process = [this](const auto& pc) {
            validate_payment_code(pc);
            notifications_.emplace(pc);
        };
        std::ranges::for_each(recipients, process);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::nym() const noexcept(false) -> const identity::Nym&
{
    if (nullptr == nym_) {
        throw std::runtime_error{
            "unable to load nym "s.append(spender_.asBase58(api_.Crypto()))};
    }

    return *nym_;
}

auto SpendPrivate::OutgoingKeys() const noexcept -> Set<crypto::Key>
{
    const auto& in = pc_recipients_;
    auto out = Set<crypto::Key>{};
    static constexpr auto key = [](const auto& i) {
        const auto& [contact, value, recipient, keyID, pubkey] = i;

        return keyID;
    };
    std::ranges::transform(in, std::inserter(out, out.end()), key);

    return out;
}

auto SpendPrivate::PasswordPrompt() const noexcept -> std::string_view
{
    return reason_.GetDisplayString();
}

auto SpendPrivate::path() const noexcept(false) -> const proto::HDPath&
{
    if (false == payment_code_path_.has_value()) {
        payment_code_path_.emplace([&] {
            auto p = proto::HDPath{};

            if (false == nym().Internal().PaymentCodePath(p)) {
                throw std::runtime_error{"failed to load path"};
            }

            return p;
        }());
    }

    return *payment_code_path_;
}

auto SpendPrivate::PaymentCodeRecipients() const noexcept
    -> std::span<const PaymentCodeRecipient>
{
    return pc_recipients_;
}

auto SpendPrivate::Policy() const noexcept -> internal::SpendPolicy
{
    return {spend_unconfirmed_incoming_, spend_unconfirmed_change_};
}

auto SpendPrivate::sender_payment_code() const noexcept(false)
    -> const PaymentCode&
{
    if (false == sender_payment_code_.has_value()) {
        sender_payment_code_.emplace(nym().PaymentCodeSecret(reason_));
    }

    return *sender_payment_code_;
}

auto SpendPrivate::SendToAddress(
    std::string_view address,
    const Amount& amount) noexcept -> bool
{
    try {
        validate_amount(amount);
        const auto [type, bytes] = validate_address(address);
        add_address(bytes.Bytes(), type, amount);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::SendToPaymentCode(
    const PaymentCode& recipient,
    const Amount& amount) noexcept -> bool
{
    try {
        validate_amount(amount);
        const auto& account = this->account(recipient);

        if (0_uz == account.OutgoingNotificationCount()) {
            notifications_.emplace(recipient);
        }

        constexpr auto subchain{crypto::Subchain::Outgoing};
        const auto i = account.Reserve(subchain, reason_);

        if (false == i.has_value()) {
            throw std::runtime_error{"Failed to allocate next key"};
        }

        const auto& key = [&]() -> const auto& {
            const auto& element = account.BalanceElement(subchain, *i);
            const auto& k = element.Key();

            if (false == k.IsValid()) {

                throw std::runtime_error{"Failed to instantiate key"};
            }

            return k;
        }();
        add_payment_code(recipient, amount, account.ID(), *i, key.PublicKey());

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::Serialize(
    proto::BlockchainTransactionProposal& out) const noexcept -> bool
{
    try {
        out.set_version(version_);
        out.set_id(id_.asBase58(api_.Crypto()));
        out.set_initiator(spender_.data(), spender_.size());
        out.set_expires(Clock::to_time_t(expires_));
        out.set_memo(memo_.data(), memo_.size());
        const auto& addr = address_recipients_;
        const auto& pc = pc_recipients_;
        auto make_output = [&, this](const auto& in) {
            this->serialize_output(in, *out.add_output());
        };
        std::ranges::for_each(addr, make_output);
        std::ranges::for_each(pc, make_output);
        const auto& notif = notifications_;
        auto make_notif = [&](const auto& in) {
            serialize_notification(
                sender_payment_code(), path(), in, *out.add_notification());
        };
        std::ranges::for_each(notif, make_notif);

        if (is_sweep()) { serialize_sweep(*out.mutable_sweep()); }

        const auto reason = reason_.GetDisplayString();
        out.set_password_prompt(reason.data(), reason.size());
        out.set_spend_unconfirmed_change(spend_unconfirmed_change_);
        out.set_spend_unconfirmed_incoming(spend_unconfirmed_incoming_);

        if (transaction_.has_value()) {
            *out.mutable_finished() = *transaction_;
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::serialize_notification(
    const PaymentCode& sender,
    const proto::HDPath& path,
    const PaymentCode& in,
    proto::BlockchainTransactionProposedNotification& out) const noexcept(false)
    -> void
{
    out.set_version(notification_version_);
    *out.mutable_path() = path;

    if (false == sender.Internal().Serialize(*out.mutable_sender())) {
        throw std::runtime_error{"failed to serialize sender payment code"};
    }

    if (false == in.Internal().Serialize(*out.mutable_recipient())) {
        throw std::runtime_error{"failed to serialize recipient payment code"};
    }
}

auto SpendPrivate::serialize_address(
    std::string_view address,
    crypto::AddressStyle type,
    proto::BlockchainTransactionProposedOutput& out) const noexcept -> void
{
    using enum crypto::AddressStyle;

    switch (type) {
        case P2WPKH: {
            out.set_segwit(true);
            [[fallthrough]];
        }
        case P2PKH: {
            out.set_pubkeyhash(address.data(), address.size());
        } break;
        case P2WSH: {
            out.set_segwit(true);
            [[fallthrough]];
        }
        case P2SH: {
            out.set_scripthash(address.data(), address.size());
        } break;
        case Unknown:
        case P2TR:
        default: {
            LogAbort()()("invalid type: ")(print(type)).Abort();
        }
    }
}

auto SpendPrivate::serialize_amount(
    const Amount& amount,
    proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
    -> void
{
    if (false == amount.Serialize(writer(out.mutable_amount()))) {
        throw std::runtime_error{"failed to serialize amount"};
    }
}

auto SpendPrivate::serialize_output(
    const AddressRecipient& in,
    proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
    -> void
{
    const auto& [contact, amount, style, bytes] = in;
    out.set_version(output_version_);
    serialize_amount(amount, out);
    serialize_address(bytes.Bytes(), style, out);
}

auto SpendPrivate::serialize_output(
    const PaymentCodeRecipient& in,
    proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
    -> void
{
    const auto& [contact, amount, recipient, keyID, pubkey] = in;
    out.set_version(output_version_);
    serialize_amount(amount, out);
    serialize_payment_code(in, out);
}

auto SpendPrivate::serialize_payment_code(
    const PaymentCodeRecipient& in,
    proto::BlockchainTransactionProposedOutput& out) const noexcept(false)
    -> void
{
    const auto& [contact, amount, recipient, keyID, pubkey] = in;
    const auto& [account, subchain, index] = keyID;
    out.set_index(index);
    out.set_paymentcodechannel(account.asBase58(api_.Crypto()));
    out.set_pubkey(UnallocatedCString{pubkey.Bytes()});
    out.set_contact(UnallocatedCString{contact.Bytes()});
}

auto SpendPrivate::serialize_sweep(
    proto::BlockchainTransactionProposedSweep& out) const noexcept(false)
    -> void
{
    out.set_version(sweep_version_);

    if (sweep_key_.has_value()) {
        auto& pKey = *out.mutable_key();
        const auto& [subaccount, subchain, index] = *sweep_key_;
        pKey.set_version(sweep_key_version_);
        pKey.set_chain(translate(UnitToClaim(blockchain_to_unit(chain_))));
        pKey.set_nym(spender_.asBase58(api_.Crypto()));
        pKey.set_subaccount(subaccount.asBase58(api_.Crypto()));
        pKey.set_subchain(static_cast<std::uint32_t>(subchain));
        pKey.set_index(index);
    } else if (sweep_subaccount_.has_value()) {
        const auto& id = sweep_subaccount_->Internal();

        if (false == id.Serialize(*out.mutable_subaccount())) {
            throw std::runtime_error{"failed to serialize subaccount id"};
        }
    }
}

auto SpendPrivate::SetMemo(std::string_view memo) noexcept -> bool
{
    memo_.assign(memo);

    return true;
}

auto SpendPrivate::SetSpendUnconfirmedChange(bool value) noexcept -> bool
{
    spend_unconfirmed_change_ = value;

    return true;
}

auto SpendPrivate::SetSpendUnconfirmedIncoming(bool value) noexcept -> bool
{
    spend_unconfirmed_incoming_ = value;

    return true;
}

auto SpendPrivate::SetSweepFromAccount(bool value) noexcept -> bool
{
    sweep_subaccount_.reset();
    sweep_key_.reset();
    using enum node::Funding;

    if (value) {
        policy_ = SweepAccount;
    } else {
        policy_ = Default;
        sweep_to_address_.reset();
    }

    return true;
}

auto SpendPrivate::SetSweepFromKey(const crypto::Key& key) noexcept -> bool
{
    const auto& owner = api_.Crypto().Blockchain().Owner(key);

    if (owner == spender_) {
        sweep_subaccount_.reset();
        sweep_key_.emplace(key);
        using enum node::Funding;
        policy_ = SweepKey;

        return true;
    } else {
        LogError()()("key is not owned by the spending nym ")(
            spender_, api_.Crypto())
            .Flush();

        return false;
    }
}

auto SpendPrivate::SetSweepFromSubaccount(
    const identifier::Account& aubaccount) noexcept -> bool
{
    const auto subaccounts =
        api_.Crypto().Blockchain().SubaccountList(spender_, chain_);

    if (subaccounts.contains(aubaccount)) {
        sweep_key_.reset();
        sweep_subaccount_.emplace(aubaccount);
        using enum node::Funding;
        policy_ = SweepSubaccount;

        return true;
    } else {
        LogError()()("subaccount ")(aubaccount, api_.Crypto())(
            " is not owned by the spending nym ")(spender_, api_.Crypto())
            .Flush();

        return false;
    }
}

auto SpendPrivate::SetSweepToAddress(std::string_view address) noexcept -> bool
{
    try {
        sweep_to_address_.reset();
        auto [type, bytes] = validate_address(address);
        sweep_to_address_.emplace(std::move(bytes), type);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SpendPrivate::SetUseEnhancedNotifications(bool value) noexcept -> bool
{
    use_enhanced_notifications_ = value;

    return true;
}

auto SpendPrivate::SpendUnconfirmedChange() const noexcept -> bool
{
    return spend_unconfirmed_change_;
}

auto SpendPrivate::SpendUnconfirmedIncoming() const noexcept -> bool
{
    return spend_unconfirmed_incoming_;
}

auto SpendPrivate::Spender() const noexcept -> const identifier::Nym&
{
    return spender_;
}

auto SpendPrivate::SweepFromAccount() const noexcept -> bool
{
    using enum node::Funding;

    return SweepAccount == policy_;
}

auto SpendPrivate::SweepFromKey() const noexcept -> const crypto::Key&
{
    if (sweep_key_.has_value()) {

        return *sweep_key_;
    } else {
        static const auto blank = crypto::Key{};

        return blank;
    }
}

auto SpendPrivate::SweepFromSubaccount() const noexcept
    -> const identifier::Account&
{
    if (sweep_subaccount_.has_value()) {

        return *sweep_subaccount_;
    } else {
        static const auto blank = identifier::Account{};

        return blank;
    }
}

auto SpendPrivate::SweepToAddress() const noexcept -> std::string_view
{
    if (sweep_to_address_.has_value()) {

        return sweep_to_address_->first.Bytes();
    } else {

        return {};
    }
}

auto SpendPrivate::UseEnhancedNotifications() const noexcept -> bool
{
    return use_enhanced_notifications_;
}

auto SpendPrivate::validate_address(std::string_view address) const
    noexcept(false) -> std::pair<crypto::AddressStyle, ByteArray>
{
    auto [data, style, chains, supported] =
        api_.Crypto().Blockchain().DecodeAddress(address);

    if ((false == chains.contains(chain_)) || (false == supported)) {
        throw std::runtime_error{"Address "s.append(address)
                                     .append(" not valid for "sv)
                                     .append(blockchain::print(chain_))};
    }

    using enum crypto::AddressStyle;

    switch (style) {
        case P2WPKH:
        case P2PKH:
        case P2WSH:
        case P2SH: {

            return std::make_pair(style, std::move(data));
        }
        case Unknown:
        case P2TR:
        default: {

            throw std::runtime_error{
                "Unsupported address type: "s.append(print(style))};
        }
    }
}

auto SpendPrivate::validate_amount(const Amount& amount) const noexcept(false)
    -> void
{
    if (0 >= amount) {
        throw std::runtime_error{"send amounts must be positive"};
    }
}

auto SpendPrivate::validate_payment_code(const PaymentCode& pc) const
    noexcept(false) -> void
{
    if (false == pc.Valid()) {
        throw std::runtime_error{
            "invalid payment code: "s.append(pc.asBase58())};
    }

    switch (pc.Version()) {
        case 3: {
        } break;
        default: {
            throw std::runtime_error{"unsupported payment code version "s
                                         .append(std::to_string(pc.Version()))
                                         .append(" for ")
                                         .append(pc.asBase58())};
        }
    }
}

// TODO if the transaction was never sent go through all the payment code
// recipients and de-allocate their output indices
SpendPrivate::~SpendPrivate() = default;
}  // namespace opentxs::blockchain::node::wallet
