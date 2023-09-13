// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <utility>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "internal/util/alloc/AllocatesChildren.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blockchain
{
class AccountCache final : public pmr::HasUpstreamAllocator,
                           public pmr::AllocatesChildren<alloc::BoostPoolUnsync>
{
public:
    auto AccountData(const identifier::Account& id) const noexcept
        -> crypto::Blockchain::AccountData;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto AccountList() const noexcept -> UnallocatedSet<identifier::Account>;
    auto SubaccountList(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto Owner(const identifier::Account& id) const noexcept
        -> const identifier::Nym&;
    auto SubaccountType(const identifier::Account& id) const noexcept -> std::
        pair<opentxs::blockchain::crypto::SubaccountType, identifier::Nym>;

    auto Populate() noexcept -> void;
    [[nodiscard]] auto RegisterAccount(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const identifier::Account& account) noexcept -> bool;
    [[nodiscard]] auto RegisterSubaccount(
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const identifier::Account& account,
        const identifier::Account& subaccount) noexcept -> bool;

    AccountCache(const api::Session& api, alloc::Default alloc) noexcept;

    ~AccountCache() final = default;

private:
    struct AccountParams final : public pmr::Allocated {
        opentxs::blockchain::Type chain_;
        identifier::Nym owner_;

        operator crypto::Blockchain::AccountData() const noexcept
        {
            return {chain_, owner_};
        }

        [[nodiscard]] auto get_deleter() noexcept -> delete_function final
        {
            return pmr::make_deleter(this);
        }

        AccountParams(
            const opentxs::blockchain::Type chain,
            const identifier::Nym& owner,
            allocator_type alloc) noexcept
            : Allocated(std::move(alloc))
            , chain_(chain)
            , owner_(owner, get_allocator())
        {
        }
    };

    struct SubaccountParams final : public pmr::Allocated {
        opentxs::blockchain::crypto::SubaccountType type_;
        opentxs::blockchain::Type chain_;
        identifier::Nym owner_;
        identifier::Account parent_account_;

        [[nodiscard]] auto get_deleter() noexcept -> delete_function final
        {
            return pmr::make_deleter(this);
        }

        SubaccountParams(
            const opentxs::blockchain::crypto::SubaccountType type,
            const opentxs::blockchain::Type chain,
            const identifier::Nym& owner,
            const identifier::Account& account,
            allocator_type alloc) noexcept
            : Allocated(std::move(alloc))
            , type_(type)
            , chain_(chain)
            , owner_(owner, get_allocator())
            , parent_account_(account, get_allocator())
        {
        }
    };

    using Subaccounts = Set<identifier::Account>;
    using NymIndex =
        Map<identifier::Nym, std::pair<identifier::Account, Subaccounts>>;
    using ChainIndex = Map<opentxs::blockchain::Type, NymIndex>;
    using Sub = Map<identifier::Account, SubaccountParams>;
    using Acct = Map<identifier::Account, AccountParams>;
    using AccountIndex = Map<identifier::Account, Set<identifier::Account>>;

    const api::Session& api_;
    bool populated_;
    ChainIndex nym_index_;
    AccountIndex account_index_;
    Acct account_params_;
    Sub subaccount_params_;
    opentxs::network::zeromq::socket::Raw socket_;

    auto build_account_map(
        const opentxs::blockchain::Type chain,
        NymIndex& map) noexcept -> void;
    auto load_nym(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& nym,
        NymIndex& output) noexcept -> void;
};
}  // namespace opentxs::api::crypto::blockchain
