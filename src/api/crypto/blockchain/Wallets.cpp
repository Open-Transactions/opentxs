// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/crypto/blockchain/Wallets.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"  // IWYU pragma: keep
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
    -> UnallocatedSet<identifier::Generic>
{
    populate();

    return index_.AccountList(nym);
}

auto Wallets::AccountList(const opentxs::blockchain::Type chain) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    populate();

    return index_.AccountList(chain);
}

auto Wallets::AccountList() const noexcept
    -> UnallocatedSet<identifier::Generic>
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

auto Wallets::LookupAccount(const identifier::Generic& id) const noexcept
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

    for (const auto chain : opentxs::blockchain::SupportedChains()) {
        get(lock, chain);
    }

    populated_ = true;
}
}  // namespace opentxs::api::crypto::blockchain
