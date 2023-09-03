// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Wallet.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainWalletKeys(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const api::crypto::Blockchain& parent,
    const blockchain::crypto::AccountIndex& index,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<blockchain::crypto::Wallet>
{
    using ReturnType = blockchain::crypto::implementation::Wallet;

    return std::make_unique<ReturnType>(api, contacts, parent, index, chain);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const api::crypto::Blockchain& parent,
    const AccountIndex& index,
    const opentxs::blockchain::Type chain) noexcept
    : parent_(parent)
    , account_index_(index)
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

    if (0 < data.index_.count(id)) { return false; }

    data.trees_.emplace_back(std::move(tree));
    const auto position = data.trees_.size() - 1_uz;
    data.index_.emplace(id, position);

    return true;
}

auto Wallet::AddHDNode(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Account& id) noexcept -> bool
{
    return get_or_create(*data_.lock(), nym)
        .Internal()
        .AddHDNode(path, standard, reason, id);
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
    const Accounts& paymentCode) const noexcept
    -> std::unique_ptr<crypto::Account>
{
    return factory::BlockchainAccountKeys(
        api_, contacts_, *this, account_index_, nym, hd, {}, paymentCode);
}

auto Wallet::get_or_create(Data& data, const identifier::Nym& id) const noexcept
    -> crypto::Account&
{
    if (false == data.index_.contains(id)) {
        auto pTree = factory(id, {}, {});

        OT_ASSERT(pTree);

        const auto added = add(data, id, std::move(pTree));

        OT_ASSERT(added);
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
        const auto pc =
            api_.Storage().Internal().Bip47ChannelsByChain(nymID, unit);
        add(data, nymID, factory(nymID, hd, pc));
    }
}

auto Wallet::size() const noexcept -> std::size_t
{
    return data_.lock_shared()->trees_.size();
}
}  // namespace opentxs::blockchain::crypto::implementation
