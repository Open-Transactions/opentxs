// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include <functional>
#include <mutex>
#include <optional>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
// }  // namespace v1
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
        -> UnallocatedSet<OTIdentifier>;
    auto New(
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::Type chain,
        const Identifier& account,
        const identifier::Nym& owner) const noexcept -> void;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym&;
    auto Type(const Identifier& accountID) const noexcept
        -> opentxs::blockchain::crypto::SubaccountType;

    auto Populate() noexcept -> void;

    AccountCache(const api::Session& api) noexcept;

private:
    using Accounts = UnallocatedSet<OTIdentifier>;
    using NymAccountMap = UnallocatedMap<OTNymID, Accounts>;
    using ChainAccountMap =
        UnallocatedMap<opentxs::blockchain::Type, std::optional<NymAccountMap>>;
    using AccountNymIndex = UnallocatedMap<OTIdentifier, OTNymID>;
    using AccountTypeIndex = UnallocatedMap<
        OTIdentifier,
        opentxs::blockchain::crypto::SubaccountType>;

    const api::Session& api_;
    mutable std::mutex lock_;
    mutable ChainAccountMap account_map_;
    mutable AccountNymIndex account_index_;
    mutable AccountTypeIndex account_type_;

    auto build_account_map(
        const Lock&,
        const opentxs::blockchain::Type chain,
        std::optional<NymAccountMap>& map) const noexcept -> void;
    auto get_account_map(const Lock&, const opentxs::blockchain::Type chain)
        const noexcept -> NymAccountMap&;
    auto load_nym(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& nym,
        NymAccountMap& output) const noexcept -> void;
};
}  // namespace opentxs::api::crypto::blockchain
