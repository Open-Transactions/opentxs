// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class AccountData;
}  // namespace proto

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc
{
class OPENTXS_EXPORT AccountData
{
public:
    auto ConfirmedBalance() const noexcept -> Amount;
    auto ConfirmedBalance_str() const noexcept -> UnallocatedCString;
    auto ID() const noexcept -> const UnallocatedCString&;
    auto Issuer() const noexcept -> const UnallocatedCString&;
    auto Name() const noexcept -> const UnallocatedCString&;
    auto Owner() const noexcept -> const UnallocatedCString&;
    auto PendingBalance() const noexcept -> Amount;
    auto PendingBalance_str() const noexcept -> UnallocatedCString;
    OPENTXS_NO_EXPORT auto Serialize(proto::AccountData& dest) const noexcept
        -> bool;
    auto Type() const noexcept -> AccountType;
    auto Unit() const noexcept -> const UnallocatedCString&;

    OPENTXS_NO_EXPORT AccountData(
        const proto::AccountData& serialized) noexcept(false);
    OPENTXS_NO_EXPORT AccountData(
        const UnallocatedCString& id,
        const UnallocatedCString& name,
        const UnallocatedCString& unit,
        const UnallocatedCString& owner,
        const UnallocatedCString& issuer,
        const UnallocatedCString& balanceS,
        const UnallocatedCString& pendingS,
        Amount balance,
        Amount pending,
        AccountType type) noexcept(false);
    AccountData() noexcept = delete;
    OPENTXS_NO_EXPORT AccountData(const AccountData&) noexcept;
    OPENTXS_NO_EXPORT AccountData(AccountData&&) noexcept;
    auto operator=(const AccountData&) -> AccountData& = delete;
    auto operator=(AccountData&&) -> AccountData& = delete;

    OPENTXS_NO_EXPORT ~AccountData();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::rpc
