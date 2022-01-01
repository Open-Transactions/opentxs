// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class AccountActivity;
class BalanceItem;
}  // namespace ui

class Amount;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT AccountActivity : virtual public List
{
public:
    using Scale = unsigned int;

    virtual auto AccountID() const noexcept -> std::string = 0;
    virtual auto Balance() const noexcept -> const Amount = 0;
    virtual auto BalancePolarity() const noexcept -> int = 0;
    virtual auto ContractID() const noexcept -> std::string = 0;
    virtual auto DepositAddress() const noexcept -> std::string = 0;
    virtual auto DepositAddress(const blockchain::Type chain) const noexcept
        -> std::string = 0;
    virtual auto DepositChains() const noexcept
        -> std::pmr::vector<blockchain::Type> = 0;
    virtual auto DisplayBalance() const noexcept -> std::string = 0;
    virtual auto DisplayUnit() const noexcept -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    virtual auto NotaryID() const noexcept -> std::string = 0;
    virtual auto NotaryName() const noexcept -> std::string = 0;
    virtual auto Send(
        const Identifier& contact,
        const Amount& amount,
        const std::string& memo = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const Identifier& contact,
        const std::string& amount,
        const std::string& memo = {},
        Scale scale = 0) const noexcept -> bool = 0;
    virtual auto Send(
        const std::string& address,
        const Amount& amount,
        const std::string& memo = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const std::string& address,
        const std::string& amount,
        const std::string& memo = {},
        Scale scale = 0) const noexcept -> bool = 0;
    virtual auto SyncPercentage() const noexcept -> double = 0;
    virtual auto SyncProgress() const noexcept -> std::pair<int, int> = 0;
    virtual auto Type() const noexcept -> AccountType = 0;
    virtual auto Unit() const noexcept -> core::UnitType = 0;
    virtual auto ValidateAddress(const std::string& text) const noexcept
        -> bool = 0;
    virtual auto ValidateAmount(const std::string& text) const noexcept
        -> std::string = 0;

    ~AccountActivity() override = default;

protected:
    AccountActivity() noexcept = default;

private:
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;
};
}  // namespace ui
}  // namespace opentxs
