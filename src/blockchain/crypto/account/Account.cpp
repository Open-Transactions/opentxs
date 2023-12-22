// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Account

#include "blockchain/crypto/account/Account.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <BlockchainEthereumAccountData.pb.h>
#include <HDAccount.pb.h>
#include <boost/endian/conversion.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/blockchain/crypto/Notification.hpp"
#include "internal/blockchain/crypto/PaymentCode.hpp"
#include "internal/blockchain/crypto/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"      // IWYU pragma: keep
#include "opentxs/blockchain/crypto/PaymentCode.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
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
            static_cast<std::underlying_type_t<blockchain::Type>>(chain);
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
using namespace std::literals;

Account::Account(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Wallet& parent,
    const identifier::Nym& nym,
    const Accounts& hd,
    const Accounts& ethereum,
    const Accounts& paymentCode) noexcept
    : api_(api)
    , contacts_(contacts)
    , parent_(parent)
    , chain_(parent.Chain())
    , nym_id_(nym)
    , account_id_(GetID(api_, nym_id_, parent.Chain()))
    , subaccounts_()
    , find_nym_([&] {
        using Dir = network::zeromq::socket::Direction;
        auto out = api_.Network().ZeroMQ().Context().Internal().PushSocket(
            Dir::Connect);
        const auto started = out->Start(api_.Endpoints().FindNym().data());

        assert_true(started);

        return out;
    }())
{
    const auto& bc = parent_.Parent().Internal();

    if (false == bc.RegisterAccount(chain_, nym_id_, account_id_)) {
        LogAbort()()("invalid account").Abort();
    }

    auto handle = subaccounts_.lock();
    auto& data = *handle;
    init_notification(data);
    init_hd(hd, data);
    init_ethereum(ethereum, data);
    init_payment_code(paymentCode, data);
}

auto Account::AddEthereum(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return init_ethereum(path, standard, reason);
}

auto Account::AddHD(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return init_hd(path, standard, reason);
}

auto Account::AddPaymentCode(
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return init_payment_code(local, remote, path, reason);
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

    using enum SubaccountType;
    const auto subaccounts = GetSubaccounts(HD);

    // Look for a BIP-44 account first
    for (const auto& a : subaccounts) {
        const auto& account = a.asDeterministic().asHD();

        if (HDProtocol::BIP_44 != account.Standard()) { continue; }

        const auto index = account.Reserve(subchain, contact, reason, label);

        if (index.has_value()) {

            return account.BalanceElement(subchain, index.value());
        }
    }

    // If no BIP-44 account exists, then use whatever else may exist
    for (const auto& a : subaccounts) {
        const auto& account = a.asDeterministic().asHD();
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
    using enum SubaccountType;
    const auto subaccounts = GetSubaccounts(PaymentCode);
    const auto extract = [&](const auto& subaccount) {
        const auto& pc = subaccount.asDeterministic().asPaymentCode();
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
    };
    std::ranges::for_each(subaccounts, extract);
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

auto Account::GetSubaccounts() const noexcept
    -> UnallocatedVector<crypto::Subaccount>
{
    return get_subaccounts(*subaccounts_.lock());
}

auto Account::GetSubaccounts(SubaccountType type) const noexcept
    -> UnallocatedVector<crypto::Subaccount>
{
    return get_subaccounts(type, *subaccounts_.lock());
}

auto Account::get_subaccounts(Data& data) const noexcept
    -> UnallocatedVector<crypto::Subaccount>
{
    using namespace std::ranges;
    const auto in = data.index_;
    auto out = UnallocatedVector<crypto::Subaccount>{};
    out.reserve(in.size());

    for (const auto& p : in) { out.emplace_back(p.second); }

    return out;
}

auto Account::get_subaccounts(SubaccountType type, Data& data) const noexcept
    -> UnallocatedVector<crypto::Subaccount>
{
    using namespace std::ranges;
    auto out = UnallocatedVector<crypto::Subaccount>{};
    const auto [begin, end] = data.map_.equal_range(type);
    const auto count = std::distance(begin, end);
    out.reserve(count);

    for (const auto& p : subrange(begin, end)) { out.emplace_back(p.second); }

    return out;
}

auto Account::init_ethereum(const Accounts& accounts, Data& data) noexcept
    -> void
{
    LogTrace()()("loading ")(accounts.size())(" ethereum subaccounts for ")(
        nym_id_, api_.Crypto())(" on ")(print(chain_))
        .Flush();
    using namespace std::ranges;
    for_each(accounts, [&, this](const auto& id) {
        init_ethereum(id, data, false);
    });
}

auto Account::init_ethereum(
    const identifier::Account& id,
    Data& data,
    bool checking) noexcept -> crypto::Subaccount&
{
    try {
        if (auto i = data.index_.find(id); data.index_.end() != i) {

            return i->second->Self();
        }

        auto proto = proto::BlockchainEthereumAccountData{};
        const auto loaded = api_.Storage().Internal().Load(nym_id_, id, proto);

        if (false == loaded) {
            if (checking) {

                return Subaccount::Blank();
            } else {
                throw std::runtime_error{
                    "failed to load serialized subaccount "s.append(
                        id.asBase58(api_.Crypto()))};
            }
        }

        auto subaccount =
            factory::BlockchainEthereumSubaccount(api_, *this, id, proto);

        if (nullptr == subaccount) {
            throw std::runtime_error{
                "failed to instantiate ethereum subaccount "s.append(
                    id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::init_ethereum(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    const auto id = api_.Factory().Internal().AccountID(
        UnitToClaim(target_to_unit(chain_)), path);
    auto handle = subaccounts_.lock();
    auto& data = *handle;

    if (auto& existing = init_ethereum(id, data, true); existing.IsValid()) {

        return existing;
    }

    try {
        auto subaccount = factory::BlockchainEthereumSubaccount(
            api_, *this, id, path, standard, reason);

        if (nullptr == subaccount) {
            throw std::runtime_error{"failed to create subaccount "s.append(
                id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::init_hd(const Accounts& accounts, Data& data) noexcept -> void
{
    LogTrace()()("loading ")(accounts.size())(" hd subaccounts for ")(
        nym_id_, api_.Crypto())(" on ")(print(chain_))
        .Flush();
    using namespace std::ranges;
    for_each(accounts, [&, this](const auto& id) { init_hd(id, data, false); });
}

auto Account::init_hd(
    const identifier::Account& id,
    Data& data,
    bool checking) noexcept -> crypto::Subaccount&
{
    try {
        if (auto i = data.index_.find(id); data.index_.end() != i) {

            return i->second->Self();
        }

        auto proto = proto::HDAccount{};
        const auto loaded = api_.Storage().Internal().Load(nym_id_, id, proto);

        if (false == loaded) {
            if (checking) {

                return Subaccount::Blank();
            } else {
                throw std::runtime_error{
                    "failed to load serialized subaccount "s.append(
                        id.asBase58(api_.Crypto()))};
            }
        }

        auto subaccount =
            factory::BlockchainHDSubaccount(api_, *this, id, proto);

        if (nullptr == subaccount) {
            throw std::runtime_error{
                "failed to instantiate subaccount "s.append(
                    id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::init_hd(
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    const auto id = api_.Factory().Internal().AccountID(
        UnitToClaim(target_to_unit(chain_)), path);
    auto handle = subaccounts_.lock();
    auto& data = *handle;

    if (auto& existing = init_hd(id, data, true); existing.IsValid()) {

        return existing;
    }

    try {
        auto subaccount = factory::BlockchainHDSubaccount(
            api_, *this, id, path, standard, reason);

        if (nullptr == subaccount) {
            throw std::runtime_error{"failed to create subaccount "s.append(
                id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::init_notification(Data& data) noexcept -> void
{
    const auto nym = api_.Wallet().Nym(nym_id_);

    assert_false(nullptr == nym);

    if (auto code = nym->PaymentCodePublic(); 0 < code.Version()) {
        const auto id = internal::Notification::CalculateID(api_, chain_, code);

        if (data.index_.contains(id)) {
            LogAbort()()(print(chain_))(
                " subchain index already contains notification account ")(
                id, api_.Crypto())
                .Abort();
        }

        auto subaccount = factory::BlockchainNotificationSubaccount(
            api_, *this, id, code, *nym);

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());
    }
}

auto Account::init_payment_code(const Accounts& accounts, Data& data) noexcept
    -> void
{
    LogTrace()()("loading ")(accounts.size())(" payment code subaccounts for ")(
        nym_id_, api_.Crypto())(" on ")(print(chain_))
        .Flush();
    using namespace std::ranges;
    for_each(accounts, [&, this](const auto& id) {
        init_payment_code(id, data, false);
    });
}

auto Account::init_payment_code(
    const identifier::Account& id,
    Data& data,
    bool checking) noexcept -> crypto::Subaccount&
{
    try {
        if (auto i = data.index_.find(id); data.index_.end() != i) {

            return i->second->Self();
        }

        auto proto = proto::Bip47Channel{};
        const auto loaded = api_.Storage().Internal().Load(nym_id_, id, proto);

        if (false == loaded) {
            if (checking) {

                return Subaccount::Blank();
            } else {
                throw std::runtime_error{
                    "failed to load serialized subaccount "s.append(
                        id.asBase58(api_.Crypto()))};
            }
        }

        auto subaccount =
            factory::BlockchainPCSubaccount(api_, contacts_, *this, id, proto);

        if (nullptr == subaccount) {
            throw std::runtime_error{
                "failed to instantiate subaccount "s.append(
                    id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::init_payment_code(
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    const auto id = internal::PaymentCode::GetID(api_, chain_, local, remote);
    auto handle = subaccounts_.lock();
    auto& data = *handle;

    if (auto& existing = init_payment_code(id, data, true);
        existing.IsValid()) {

        return existing;
    }

    try {
        auto subaccount = factory::BlockchainPCSubaccount(
            api_, contacts_, *this, id, local, remote, path, reason);

        if (nullptr == subaccount) {
            throw std::runtime_error{"failed to create subaccount "s.append(
                id.asBase58(api_.Crypto()))};
        }

        data.map_.emplace(subaccount->Type(), subaccount);
        data.index_.emplace(id, subaccount);

        assert_true(data.map_.size() == data.index_.size());

        return subaccount->Self();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return Subaccount::Blank();
    }
}

auto Account::Subaccount(const identifier::Account& id) const noexcept(false)
    -> crypto::Subaccount&
{
    return const_cast<Account*>(this)->Subaccount(id);
}

auto Account::Subaccount(const identifier::Account& id) noexcept(false)
    -> crypto::Subaccount&
{
    const auto handle = subaccounts_.lock_shared();
    const auto& index = handle->index_;

    if (auto i = index.find(id); index.end() != i) {

        return i->second->Self();
    } else {

        throw std::out_of_range("account "s.append(id.asBase58(api_.Crypto()))
                                    .append(" not found"));
    }
}
}  // namespace opentxs::blockchain::crypto::implementation
