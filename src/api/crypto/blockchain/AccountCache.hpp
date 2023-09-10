// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
class AccountCache
{
public:
    auto List(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto Owner(const identifier::Account& accountID) const noexcept
        -> const identifier::Nym&;
    auto Type(const identifier::Account& accountID) const noexcept
        -> opentxs::blockchain::crypto::SubaccountType;

    auto Populate() noexcept -> void;
    [[nodiscard]] auto RegisterSubaccount(
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const identifier::Account& account,
        const identifier::Account& subaccount) noexcept -> bool;

    AccountCache(const api::Session& api) noexcept;

private:
    struct SubaccountParams {
        opentxs::blockchain::crypto::SubaccountType type_{};
        opentxs::blockchain::Type chain_{};
        identifier::Nym owner_{};
        identifier::Account parent_account_{};

        SubaccountParams(
            const opentxs::blockchain::crypto::SubaccountType type,
            const opentxs::blockchain::Type chain,
            const identifier::Nym& owner,
            const identifier::Account& account) noexcept
            : type_(type)
            , chain_(chain)
            , owner_(owner)
            , parent_account_(account)
        {
        }
    };

    using Subaccounts = UnallocatedSet<identifier::Account>;
    using NymIndex = Map<identifier::Nym, Subaccounts>;
    using ChainIndex = Map<opentxs::blockchain::Type, NymIndex>;
    using Params = UnallocatedMap<identifier::Account, SubaccountParams>;

    const api::Session& api_;
    bool populated_;
    ChainIndex index_;
    Params params_;
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
