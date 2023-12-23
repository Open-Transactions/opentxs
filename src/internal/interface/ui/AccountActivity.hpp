// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <string_view>
#include <tuple>

#include "internal/interface/ui/List.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/display/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountActivity;
class BalanceItem;
}  // namespace ui

class Amount;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages a set of rows containing Balance Items for the account.
  It is also an easy way to access account-level data such as balance or
  receiving address.
*/
class AccountActivity : virtual public List
{
public:
    using Scale = display::SpecifiedScale;

    /// returns the account ID.
    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    /// returns the account's balance as an Amount object.
    virtual auto Balance() const noexcept -> const Amount = 0;
    /// returns Polarity, since the account balance can be positive or negative.
    virtual auto BalancePolarity() const noexcept -> int = 0;
    /// returns For off-chain assets, returns the unit definition ID for the
    /// associated asset contract.
    virtual auto ContractID() const noexcept -> UnallocatedCString = 0;
    ///@{
    /**
       @name DepositAddress
       @return A new receiving address.
    */
    virtual auto DepositAddress() const noexcept -> UnallocatedCString = 0;
    virtual auto DepositAddress(const blockchain::Type chain) const noexcept
        -> UnallocatedCString = 0;
    ///@}
    virtual auto DepositChains() const noexcept
        -> UnallocatedVector<blockchain::Type> = 0;
    /// returns a string containing the account balance formatted for display in
    /// the UI.
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    /// returns a string containing the account's unit type formatted for
    /// display in the UI.
    virtual auto DisplayUnit() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    /// returns Display name for the account.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BalanceItem> = 0;
    /// For off-chain assets, this returns the off-chain account's Notary ID.
    virtual auto NotaryID() const noexcept -> UnallocatedCString = 0;
    /// For off-chain assets, this returns the off-chain account's display name.
    virtual auto NotaryName() const noexcept -> UnallocatedCString = 0;
    virtual auto Notify(std::span<const PaymentCode> contacts) const noexcept
        -> bool = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
    ///@{
    /**
       @name Send
       Send funds from the account to an address or contact.
       @param address Recipient address as string.
       @param amount Amount as object or string.
       @param contact Identifier containing recipient's Contact ID.
       @param memo Optional memo field as string.
       @return Whether or not the send was successful.
    */
    virtual auto Send(
        const identifier::Generic& contact,
        const Amount& amount,
        const std::string_view memo = {},
        std::span<const PaymentCode> notify = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const identifier::Generic& contact,
        const UnallocatedCString& amount,
        const std::string_view memo = {},
        Scale scale = std::nullopt,
        std::span<const PaymentCode> notify = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const UnallocatedCString& address,
        const Amount& amount,
        const std::string_view memo = {},
        std::span<const PaymentCode> notify = {}) const noexcept -> bool = 0;
    virtual auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const std::string_view memo = {},
        Scale scale = std::nullopt,
        std::span<const PaymentCode> notify = {}) const noexcept -> bool = 0;
    ///@}
#pragma GCC diagnostic pop
    /// returns Account's current synchronization progress.
    virtual auto SyncPercentage() const noexcept -> double = 0;
    /// returns Account's ot::blockchain::Type and related current
    /// synchronization progress.
    virtual auto SyncProgress() const noexcept -> std::pair<int, int> = 0;
    /// Off-chain accounts are type 1, issuer accounts are type 2, and
    /// blockchain accounts are type 3.
    virtual auto Type() const noexcept -> AccountType = 0;
    /// returns UnitType for the account.
    virtual auto Unit() const noexcept -> UnitType = 0;
    /// returns true if the supplied address text is valid recipient for sends
    /// originating from this account.
    virtual auto ValidateAddress(const UnallocatedCString& text) const noexcept
        -> bool = 0;
    /// Input is an amount string from user input. Output is the re-formatted
    /// amount string.
    virtual auto ValidateAmount(const UnallocatedCString& text) const noexcept
        -> UnallocatedCString = 0;

    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;

    ~AccountActivity() override = default;

protected:
    AccountActivity() noexcept = default;
};
}  // namespace opentxs::ui
