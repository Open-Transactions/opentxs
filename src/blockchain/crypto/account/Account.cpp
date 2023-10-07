// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Account

#include "blockchain/crypto/account/Account.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <HDAccount.pb.h>
#include <boost/endian/conversion.hpp>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/blockchain/crypto/PaymentCode.hpp"
#include "internal/blockchain/crypto/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::factory
{
auto BlockchainAccountKeys(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const blockchain::crypto::Wallet& parent,
    const identifier::Nym& id,
    const UnallocatedSet<identifier::Account>& hd,
    const UnallocatedSet<identifier::Account>& imported,
    const UnallocatedSet<identifier::Account>& pc) noexcept
    -> std::unique_ptr<blockchain::crypto::Account>
{
    using ReturnType = blockchain::crypto::implementation::Account;

    return std::make_unique<ReturnType>(
        api, contacts, parent, id, hd, imported, pc);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::internal
{
auto Account::GetID(
    const api::Session& api,
    const identifier::Nym& owner,
    blockchain::Type chain) noexcept -> identifier::Account
{
    const auto preimage = [&] {
        auto out = api.Factory().DataFromBytes(owner.Bytes());
        auto type =
            static_cast<std::underlying_type<blockchain::Type>::type>(chain);
        boost::endian::native_to_little_inplace(type);
        out.Concatenate(&type, sizeof(type));

        return out;
    }();
    using enum identifier::AccountSubtype;

    return api.Factory().AccountIDFromPreimage(
        preimage.Bytes(), blockchain_account);
}
}  // namespace opentxs::blockchain::crypto::internal

namespace opentxs::blockchain::crypto::implementation
{
Account::Account(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Wallet& parent,
    const identifier::Nym& nym,
    const Accounts& hd,
    const Accounts& imported,
    const Accounts& paymentCode) noexcept
    : api_(api)
    , contacts_(contacts)
    , parent_(parent)
    , chain_(parent.Chain())
    , nym_id_(nym)
    , account_id_(GetID(api_, nym_id_, parent.Chain()))
    , hd_(api_, SubaccountType::HD, *this)
    , imported_(api_, SubaccountType::Imported, *this)
    , notification_(api_, SubaccountType::Notification, *this)
    , payment_code_(api_, SubaccountType::PaymentCode, *this)
    , node_index_()
    , find_nym_([&] {
        using Dir = network::zeromq::socket::Direction;
        auto out = api_.Network().ZeroMQ().Internal().PushSocket(Dir::Connect);
        const auto started = out->Start(api_.Endpoints().FindNym().data());

        assert_true(started);

        return out;
    }())
{
    const auto& bc = parent_.Parent().Internal();

    if (false == bc.RegisterAccount(chain_, nym_id_, account_id_)) {
        LogAbort()()("invalid account").Abort();
    }

    init_notification();
    init_hd(hd);
    init_payment_code(paymentCode);
}

auto Account::AddHDNode(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Account& id) noexcept -> bool
{
    return init_hd(path, standard, reason, id);
}

auto Account::AddOrUpdatePaymentCode(
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason,
    identifier::Account& out) noexcept -> bool
{
    out = internal::PaymentCode::GetID(api_, chain_, local, remote);

    return init_payment_code(local, remote, path, reason, out);
}

auto Account::ClaimAccountID(
    const identifier::Account& id,
    bool existing,
    crypto::Subaccount* node) const noexcept -> bool
{
    if (node_index_.Add(api_.Crypto(), id, node)) {

        return parent_.Parent().Internal().RegisterSubaccount(
                   node->Type(), chain_, nym_id_, account_id_, id) ==
               (!existing);
    } else {

        return false;
    }
}

auto Account::find_next_element(
    Subchain subchain,
    const identifier::Generic& contact,
    const UnallocatedCString& label,
    const PasswordPrompt& reason) const noexcept(false)
    -> const crypto::Element&
{
    // TODO Add a mechanism for setting a default subaccount in case more than
    // one is present. Also handle cases where only an imported subaccount
    // exists

    // Look for a BIP-44 account first
    for (const auto& account : hd_) {
        if (HDProtocol::BIP_44 != account.Standard()) { continue; }

        const auto index = account.Reserve(subchain, contact, reason, label);

        if (index.has_value()) {

            return account.BalanceElement(subchain, index.value());
        }
    }

    // If no BIP-44 account exists, then use whatever else may exist
    for (const auto& account : hd_) {
        const auto index = account.Reserve(subchain, contact, reason, label);

        if (index.has_value()) {

            return account.BalanceElement(subchain, index.value());
        }
    }

    throw std::runtime_error("No available element for selected subchain");
}

auto Account::FindNym(const identifier::Nym& id) const noexcept -> void
{
    find_nym_->Send([&] {
        auto work =
            network::zeromq::tagged_message(WorkType::OTXSearchNym, true);
        work.AddFrame(id);

        return work;
    }());
}

auto Account::Get(Notifications& out) const noexcept -> void
{
    payment_code_.for_each([&](const crypto::PaymentCode& pc) {
        const auto [incoming, outgoing] = pc.NotificationCount();
        const auto& remote = pc.Remote();
        auto atLeastOne{false};

        if (0_uz < incoming) {
            out.incoming_.emplace(remote);
            atLeastOne = true;
        }

        if (0_uz < outgoing) {
            out.outgoing_.emplace(remote);
            atLeastOne = true;
        }

        if (false == atLeastOne) { out.neither_.emplace(remote); }
    });
}

auto Account::GetNextChangeKey(const PasswordPrompt& reason) const
    noexcept(false) -> const crypto::Element&
{
    static const auto blank = identifier::Generic{};

    return find_next_element(Subchain::Internal, blank, "", reason);
}

auto Account::GetNextDepositKey(const PasswordPrompt& reason) const
    noexcept(false) -> const crypto::Element&
{
    static const auto blank = identifier::Generic{};

    return find_next_element(Subchain::External, blank, "", reason);
}

auto Account::GetDepositAddress(
    const blockchain::crypto::AddressStyle style,
    const PasswordPrompt& reason,
    const UnallocatedCString& memo) const noexcept -> UnallocatedCString
{
    static const auto blank = identifier::Generic{};

    return GetDepositAddress(style, blank, reason, memo);
}

auto Account::GetDepositAddress(
    const blockchain::crypto::AddressStyle style,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const UnallocatedCString& memo) const noexcept -> UnallocatedCString
{
    const auto& element =
        find_next_element(Subchain::External, contact, memo, reason);

    return element.Address(style);
}

auto Account::init_hd(const Accounts& accounts) noexcept -> void
{
    LogTrace()()("loading ")(accounts.size())(" hd subaccounts for ")(
        nym_id_, api_.Crypto())(" on ")(print(chain_))
        .Flush();

    for (const auto& id : accounts) {
        auto copy = id;
        init_hd(copy);
    }
}

auto Account::init_hd(identifier::Account& id) noexcept -> bool
{
    auto proto = proto::HDAccount{};
    const auto loaded = api_.Storage().Internal().Load(nym_id_, id, proto);

    if (loaded) {

        return hd_.Construct(id, proto);
    } else {

        return false;
    }
}

auto Account::init_hd(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Account& id) noexcept -> bool
{
    if (init_hd(id)) { return true; }

    return hd_.Construct(id, path, standard, reason);
}

auto Account::init_notification() noexcept -> void
{
    const auto nym = api_.Wallet().Nym(nym_id_);

    assert_false(nullptr == nym);

    if (auto code = nym->PaymentCodePublic(); 0 < code.Version()) {
        auto notUsed = identifier::Account{};
        notification_.Construct(notUsed, code, *nym);
    }
}

auto Account::init_payment_code(const Accounts& accounts) noexcept -> void
{
    LogTrace()()("loading ")(accounts.size())(" payment code subaccounts for ")(
        nym_id_, api_.Crypto())(" on ")(print(chain_))
        .Flush();

    for (const auto& id : accounts) {
        auto copy = id;
        init_payment_code(copy);
    }
}

auto Account::init_payment_code(identifier::Account& id) noexcept -> bool
{
    auto proto = proto::Bip47Channel{};
    const auto loaded = api_.Storage().Internal().Load(nym_id_, id, proto);

    if (loaded) {

        return payment_code_.Construct(id, contacts_, proto);
    } else {

        return false;
    }
}

auto Account::init_payment_code(
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason,
    identifier::Account& id) noexcept -> bool
{
    if (init_payment_code(id)) { return true; }

    return payment_code_.Construct(id, contacts_, local, remote, path, reason);
}

auto Account::Subaccount(const identifier::Account& id) const noexcept(false)
    -> const crypto::Subaccount&
{
    return const_cast<Account*>(this)->Subaccount(id);
}

auto Account::Subaccount(const identifier::Account& id) noexcept(false)
    -> crypto::Subaccount&
{
    auto* output = node_index_.Find(id);

    if (nullptr == output) { throw std::out_of_range("Account not found"); }

    return *output;
}
}  // namespace opentxs::blockchain::crypto::implementation
