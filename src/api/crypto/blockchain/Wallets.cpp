// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Wallets.hpp"  // IWYU pragma: associated

#include <span>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::api::crypto::blockchain
{
Wallets::Wallets(
    const api::Session& api,
    const api::session::Contacts& contacts,
    api::crypto::Blockchain& parent) noexcept
    : api_(api)
    , contacts_(contacts)
    , parent_(parent)
    , index_(api_)
    , lock_()
    , populated_(false)
    , lists_()
{
}

auto Wallets::AccountList(const identifier::Nym& nym) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    populate();

    return index_.AccountList(nym);
}

auto Wallets::AccountList(const opentxs::blockchain::Type chain) const noexcept
    -> UnallocatedSet<identifier::Account>
{
    populate();

    return index_.AccountList(chain);
}

auto Wallets::AccountList() const noexcept
    -> UnallocatedSet<identifier::Account>
{
    populate();

    return index_.AccountList();
}

auto Wallets::Get(const opentxs::blockchain::Type chain) noexcept
    -> opentxs::blockchain::crypto::Wallet&
{
    auto lock = Lock{lock_};

    return get(lock, chain);
}

auto Wallets::get(const Lock& lock, const opentxs::blockchain::Type chain)
    const noexcept -> opentxs::blockchain::crypto::Wallet&
{
    auto it = lists_.find(chain);

    if (lists_.end() != it) { return *it->second; }

    auto [it2, added] = lists_.emplace(
        chain,
        factory::BlockchainWalletKeys(api_, contacts_, parent_, index_, chain));

    OT_ASSERT(added);
    OT_ASSERT(it2->second);

    return *it2->second;
}

auto Wallets::LookupAccount(const identifier::Account& id) const noexcept
    -> AccountData
{
    populate();

    return index_.Query(id);
}

auto Wallets::populate() const noexcept -> void
{
    auto lock = Lock{lock_};
    populate(lock);
}

auto Wallets::populate(const Lock& lock) const noexcept -> void
{
    if (populated_) { return; }

    const auto nyms = api_.Storage().LocalNyms();
    auto exists = Set<opentxs::blockchain::Type>{};

    for (const auto chain : opentxs::blockchain::supported_chains()) {
        const auto unit = blockchain_to_unit(chain);

        for (const auto& nymID : nyms) {
            const auto hd =
                api_.Storage().Internal().BlockchainAccountList(nymID, unit);
            const auto pc =
                api_.Storage().Internal().Bip47ChannelsByChain(nymID, unit);

            if ((false == hd.empty()) || (false == pc.empty())) {
                exists.emplace(chain);
            }
        }
    }

    for (const auto chain : exists) { get(lock, chain); }

    populated_ = true;
}
}  // namespace opentxs::api::crypto::blockchain
