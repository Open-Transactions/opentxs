// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/wallet/Wallet.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainWalletKeys(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const api::crypto::Blockchain& parent,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<blockchain::crypto::Wallet>
{
    using ReturnType = blockchain::crypto::implementation::Wallet;

    return std::make_unique<ReturnType>(api, contacts, parent, chain);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const api::crypto::Blockchain& parent,
    const opentxs::blockchain::Type chain) noexcept
    : parent_(parent)
    , api_(api)
    , contacts_(contacts)
    , chain_(chain)
    , data_()
{
    init();
}

auto Wallet::Account(const identifier::Nym& id) const noexcept
    -> crypto::Account&
{
    return get_or_create(*data_.lock(), id);
}

auto Wallet::add(
    Data& data,
    const identifier::Nym& id,
    std::unique_ptr<crypto::Account> tree) const noexcept -> bool
{
    if (false == bool(tree)) { return false; }

    if (data.index_.contains(id)) { return false; }

    data.trees_.emplace_back(std::move(tree));
    const auto position = data.trees_.size() - 1_uz;
    data.index_.emplace(id, position);

    return true;
}

auto Wallet::AddEthereum(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return get_or_create(*data_.lock(), nym)
        .Internal()
        .AddEthereum(path, standard, reason);
}

auto Wallet::AddHD(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return get_or_create(*data_.lock(), nym)
        .Internal()
        .AddHD(path, standard, reason);
}

auto Wallet::AddPaymentCode(
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason) noexcept -> crypto::Subaccount&
{
    return get_or_create(*data_.lock(), local.ID())
        .Internal()
        .AddPaymentCode(local, remote, path, reason);
}

auto Wallet::at(const std::size_t position) const noexcept(false)
    -> Wallet::const_iterator::value_type&
{
    return at(*data_.lock_shared(), position);
}

auto Wallet::at(const Data& data, const std::size_t index) const noexcept(false)
    -> const_iterator::value_type&
{
    return *data.trees_.at(index);
}

auto Wallet::at(Data& data, const std::size_t index) const noexcept(false)
    -> crypto::Account&
{
    return *data.trees_.at(index);
}

auto Wallet::cend() const noexcept -> const_iterator
{
    return {this, data_.lock_shared()->trees_.size()};
}

auto Wallet::end() const noexcept -> const_iterator
{
    return {this, data_.lock_shared()->trees_.size()};
}

auto Wallet::factory(
    const identifier::Nym& nym,
    const Accounts& hd,
    const Accounts& ethereum,
    const Accounts& paymentCode) const noexcept
    -> std::unique_ptr<crypto::Account>
{
    return factory::BlockchainAccountKeys(
        api_, contacts_, *this, nym, hd, ethereum, paymentCode);
}

auto Wallet::get_or_create(Data& data, const identifier::Nym& id) const noexcept
    -> crypto::Account&
{
    if (false == data.index_.contains(id)) {
        auto pTree = factory(id, {}, {}, {});

        assert_false(nullptr == pTree);

        const auto added = add(data, id, std::move(pTree));

        assert_true(added);
    }

    return at(data, data.index_.at(id));
}

auto Wallet::init() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto nyms = api_.Storage().LocalNyms();

    for (const auto& nymID : nyms) {
        const auto unit = blockchain_to_unit(chain_);
        const auto hd =
            api_.Storage().Internal().BlockchainAccountList(nymID, unit);
        const auto ethereum =
            api_.Storage().Internal().BlockchainEthereumAccountList(
                nymID, unit);
        const auto pc =
            api_.Storage().Internal().Bip47ChannelsByChain(nymID, unit);
        LogConsole()("Loading ")(print(chain_))(" key manager for ")(
            nymID, api_.Crypto())(" with ")(hd.size())(" HD subaccounts, ")(
            ethereum.size())(" ethereum subaccounts, ")(" and ")(pc.size())(
            " payment code subaccounts")
            .Flush();
        add(data, nymID, factory(nymID, hd, ethereum, pc));
    }
}

auto Wallet::size() const noexcept -> std::size_t
{
    return data_.lock_shared()->trees_.size();
}
}  // namespace opentxs::blockchain::crypto::implementation
