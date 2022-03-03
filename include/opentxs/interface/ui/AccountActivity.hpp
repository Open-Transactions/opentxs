// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class AccountActivity;
class BalanceItem;
}  // namespace ui

class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT AccountActivity : virtual public List
{
public:
    using Scale = unsigned int;

    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    virtual auto Balance() const noexcept -> const Amount = 0;
    virtual auto BalancePolarity() const noexcept -> int = 0;
    virtual auto ContractID() const noexcept -> UnallocatedCString = 0;
    virtual auto DepositAddress() const noexcept -> UnallocatedCString = 0;
    virtual auto DepositAddress(const blockchain::Type chain) const noexcept
        -> UnallocatedCString = 0;
    virtual auto DepositChains() const noexcept
        -> UnallocatedVector<blockchain::Type> = 0;
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    virtual auto DisplayUnit() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    virtual auto NotaryID() const noexcept -> UnallocatedCString = 0;
    virtual auto NotaryName() const noexcept -> UnallocatedCString = 0;
    virtual auto Send(
        const Identifier& contact,
        const Amount& amount,
        const UnallocatedCString& memo = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const Identifier& contact,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo = {},
        Scale scale = 0) const noexcept -> bool = 0;
    virtual auto Send(
        const UnallocatedCString& address,
        const Amount& amount,
        const UnallocatedCString& memo = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo = {},
        Scale scale = 0) const noexcept -> bool = 0;
    virtual auto SyncPercentage() const noexcept -> double = 0;
    virtual auto SyncProgress() const noexcept -> std::pair<int, int> = 0;
    virtual auto Type() const noexcept -> AccountType = 0;
    virtual auto Unit() const noexcept -> UnitType = 0;
    virtual auto ValidateAddress(const UnallocatedCString& text) const noexcept
        -> bool = 0;
    virtual auto ValidateAmount(const UnallocatedCString& text) const noexcept
        -> UnallocatedCString = 0;

    ~AccountActivity() override = default;

protected:
    AccountActivity() noexcept = default;

private:
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;
};
}  // namespace opentxs::ui
