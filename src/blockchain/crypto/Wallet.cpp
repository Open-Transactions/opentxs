// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/crypto/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "blockchain/crypto/AccountIndex.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"

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
    , lock_()
    , trees_()
    , index_()
{
    init();
}

auto Wallet::Account(const identifier::Nym& id) const noexcept
    -> crypto::Account&
{
    Lock lock(lock_);

    return const_cast<Wallet&>(*this).get_or_create(lock, id);
}

auto Wallet::add(
    const Lock& lock,
    const identifier::Nym& id,
    std::unique_ptr<crypto::Account> tree) noexcept -> bool
{
    if (false == bool(tree)) { return false; }

    if (0 < index_.count(id)) { return false; }

    trees_.emplace_back(std::move(tree));
    const std::size_t position = trees_.size() - 1;
    index_.emplace(id, position);

    return true;
}

auto Wallet::AddHDNode(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    const crypto::HDProtocol standard,
    const PasswordPrompt& reason,
    identifier::Generic& id) noexcept -> bool
{
    Lock lock(lock_);

    return get_or_create(lock, nym).Internal().AddHDNode(
        path, standard, reason, id);
}

auto Wallet::at(const std::size_t position) const noexcept(false)
    -> Wallet::const_iterator::value_type&
{
    Lock lock(lock_);

    return at(lock, position);
}

auto Wallet::at(const Lock& lock, const std::size_t index) const noexcept(false)
    -> const_iterator::value_type&
{
    return *trees_.at(index);
}

auto Wallet::at(const Lock& lock, const std::size_t index) noexcept(false)
    -> crypto::Account&
{
    return *trees_.at(index);
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

auto Wallet::get_or_create(const Lock& lock, const identifier::Nym& id) noexcept
    -> crypto::Account&
{
    if (0 == index_.count(id)) {
        auto pTree = factory(id, {}, {});

        OT_ASSERT(pTree);

        const auto added = add(lock, id, std::move(pTree));

        OT_ASSERT(added);
    }

    return at(lock, index_.at(id));
}

void Wallet::init() noexcept
{
    Lock lock(lock_);
    const auto nyms = api_.Storage().LocalNyms();

    for (const auto& nymID : nyms) {
        const auto hdAccounts = [&] {
            auto out = Accounts{};
            const auto list = api_.Storage().BlockchainAccountList(
                nymID, BlockchainToUnit(chain_));
            std::transform(
                list.begin(),
                list.end(),
                std::inserter(out, out.end()),
                [&](const auto& in) {
                    return api_.Factory().IdentifierFromBase58(in);
                });

            return out;
        }();
        const auto pcAccounts = api_.Storage().Bip47ChannelsByChain(
            nymID, BlockchainToUnit(chain_));

        add(lock, nymID, factory(nymID, hdAccounts, pcAccounts));
    }
}
}  // namespace opentxs::blockchain::crypto::implementation
