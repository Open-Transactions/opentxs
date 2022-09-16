// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/crypto/AccountIndex.hpp"  // IWYU pragma: associated

#include <memory>
#include <shared_mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::crypto
{
struct AccountIndex::Imp {
    using Accounts = UnallocatedSet<identifier::Generic>;

    auto AccountList(const identifier::Nym& nymID) const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        try {

            return nym_index_.at(nymID);
        } catch (...) {

            return {};
        }
    }
    auto AccountList(const Chain chain) const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        try {

            return chain_index_.at(chain);
        } catch (...) {

            return {};
        }
    }
    auto AccountList() const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        return all_;
    }
    auto Query(const identifier::Generic& account) const noexcept -> Data
    {
        auto lock = sLock{lock_};

        try {

            return map_.at(account);
        } catch (...) {

            return blank_;
        }
    }
    auto Register(
        const identifier::Generic& account,
        const identifier::Nym& owner,
        Chain chain) const noexcept -> void
    {
        auto lock = eLock{lock_};
        map_.try_emplace(account, chain, owner);
        chain_index_[chain].emplace(account);
        nym_index_[owner].emplace(account);
        all_.emplace(account);
    }

    Imp(const api::Session& api) noexcept
        : blank_(Chain::Unknown, identifier::Nym{})
        , lock_()
        , map_()
        , chain_index_()
        , nym_index_()
        , all_()
    {
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    using Map = UnallocatedMap<identifier::Generic, Data>;
    using ChainIndex = UnallocatedMap<Chain, Accounts>;
    using NymIndex = UnallocatedMap<identifier::Nym, Accounts>;

    const Data blank_;
    mutable std::shared_mutex lock_;
    mutable Map map_;
    mutable ChainIndex chain_index_;
    mutable NymIndex nym_index_;
    mutable Accounts all_;
};

AccountIndex::AccountIndex(const api::Session& api) noexcept
    : imp_(std::make_unique<Imp>(api).release())
{
}

auto AccountIndex::AccountList(const identifier::Nym& nymID) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return imp_->AccountList(nymID);
}

auto AccountIndex::AccountList(const Chain chain) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return imp_->AccountList(chain);
}

auto AccountIndex::AccountList() const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return imp_->AccountList();
}

auto AccountIndex::Query(const identifier::Generic& account) const noexcept
    -> Data
{
    return imp_->Query(account);
}

auto AccountIndex::Register(
    const identifier::Generic& account,
    const identifier::Nym& owner,
    Chain chain) const noexcept -> void
{
    imp_->Register(account, owner, chain);
}

AccountIndex::~AccountIndex()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::crypto
