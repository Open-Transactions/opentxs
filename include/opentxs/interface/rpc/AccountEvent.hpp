// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::rpc::AccountEventType
// IWYU pragma: no_include "opentxs/interface/rpc/AccountEventType.hpp"

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class AccountEvent;
}  // namespace proto
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc
{
class OPENTXS_EXPORT AccountEvent
{
public:
    auto AccountID() const noexcept -> const UnallocatedCString&;
    auto ConfirmedAmount() const noexcept -> Amount;
    auto ConfirmedAmount_str() const noexcept -> const UnallocatedCString&;
    auto ContactID() const noexcept -> const UnallocatedCString&;
    auto Memo() const noexcept -> const UnallocatedCString&;
    auto PendingAmount() const noexcept -> Amount;
    auto PendingAmount_str() const noexcept -> const UnallocatedCString&;
    OPENTXS_NO_EXPORT auto Serialize(proto::AccountEvent& dest) const noexcept
        -> bool;
    auto State() const noexcept -> int;
    auto Timestamp() const noexcept -> Time;
    auto Type() const noexcept -> AccountEventType;
    auto UUID() const noexcept -> const UnallocatedCString&;
    auto WorkflowID() const noexcept -> const UnallocatedCString&;

    OPENTXS_NO_EXPORT AccountEvent(
        const proto::AccountEvent& serialized) noexcept(false);
    OPENTXS_NO_EXPORT AccountEvent(
        const UnallocatedCString& account,
        AccountEventType type,
        const UnallocatedCString& contact,
        const UnallocatedCString& workflow,
        const UnallocatedCString& amountS,
        const UnallocatedCString& pendingS,
        Amount amount,
        Amount pending,
        opentxs::Time time,
        const UnallocatedCString& memo,
        const UnallocatedCString& uuid,
        int state) noexcept(false);
    AccountEvent() noexcept = delete;
    OPENTXS_NO_EXPORT AccountEvent(const AccountEvent&) noexcept;
    OPENTXS_NO_EXPORT AccountEvent(AccountEvent&&) noexcept;
    auto operator=(const AccountEvent&) -> AccountEvent& = delete;
    auto operator=(AccountEvent&&) -> AccountEvent& = delete;

    OPENTXS_NO_EXPORT ~AccountEvent();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::rpc
