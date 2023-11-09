// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Account;
}  // namespace internal

class Element;
class HD;
class Imported;
class Notification;
class PaymentCode;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Account;
class Generic;
class Nym;
}  // namespace identifier

class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Account
{
public:
    virtual auto AccountID() const noexcept -> const identifier::Account& = 0;
    virtual auto GetDepositAddress(
        const AddressStyle style,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo = "") const noexcept
        -> UnallocatedCString = 0;
    virtual auto GetDepositAddress(
        const AddressStyle style,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo = "") const noexcept
        -> UnallocatedCString = 0;
    /// Throws std::out_of_range if no keys are available
    virtual auto GetNextChangeKey(const PasswordPrompt& reason) const
        noexcept(false) -> const Element& = 0;
    /// Throws std::out_of_range if no keys are available
    virtual auto GetNextDepositKey(const PasswordPrompt& reason) const
        noexcept(false) -> const Element& = 0;
    virtual auto GetSubaccounts() const noexcept
        -> UnallocatedVector<opentxs::blockchain::crypto::Subaccount> = 0;
    virtual auto GetSubaccounts(SubaccountType type) const noexcept
        -> UnallocatedVector<opentxs::blockchain::crypto::Subaccount> = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> internal::Account& = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Parent() const noexcept -> const Wallet& = 0;
    virtual auto Subaccount(const identifier::Account& id) const noexcept(false)
        -> crypto::Subaccount& = 0;
    virtual auto Target() const noexcept -> crypto::Target = 0;

    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    OPENTXS_NO_EXPORT virtual ~Account() = default;

protected:
    Account() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
