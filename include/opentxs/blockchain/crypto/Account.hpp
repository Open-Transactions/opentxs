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
    struct OPENTXS_EXPORT HDAccounts {
        using value_type = HD;
        using const_iterator = opentxs::iterator::
            Bidirectional<const HDAccounts, const value_type>;

        virtual auto all() const noexcept
            -> UnallocatedSet<identifier::Account> = 0;
        /// Throws std::out_of_range for invalid position
        virtual auto at(const std::size_t position) const noexcept(false)
            -> const value_type& = 0;
        /// Throws std::out_of_range for invalid id
        virtual auto at(const identifier::Account& id) const noexcept(false)
            -> const value_type& = 0;
        virtual auto begin() const noexcept -> const_iterator = 0;
        virtual auto cbegin() const noexcept -> const_iterator = 0;
        virtual auto cend() const noexcept -> const_iterator = 0;
        virtual auto end() const noexcept -> const_iterator = 0;
        virtual auto size() const noexcept -> std::size_t = 0;
        virtual auto Type() const noexcept -> SubaccountType = 0;

        OPENTXS_NO_EXPORT virtual ~HDAccounts() = default;
    };
    struct OPENTXS_EXPORT ImportedAccounts {
        using value_type = Imported;
        using const_iterator = opentxs::iterator::
            Bidirectional<const ImportedAccounts, const value_type>;

        virtual auto all() const noexcept
            -> UnallocatedSet<identifier::Account> = 0;
        /// Throws std::out_of_range for invalid position
        virtual auto at(const std::size_t position) const noexcept(false)
            -> const value_type& = 0;
        /// Throws std::out_of_range for invalid id
        virtual auto at(const identifier::Account& id) const noexcept(false)
            -> const value_type& = 0;
        virtual auto begin() const noexcept -> const_iterator = 0;
        virtual auto cbegin() const noexcept -> const_iterator = 0;
        virtual auto cend() const noexcept -> const_iterator = 0;
        virtual auto end() const noexcept -> const_iterator = 0;
        virtual auto size() const noexcept -> std::size_t = 0;
        virtual auto Type() const noexcept -> SubaccountType = 0;

        OPENTXS_NO_EXPORT virtual ~ImportedAccounts() = default;
    };
    struct OPENTXS_EXPORT NotificationAccounts {
        using value_type = Notification;
        using const_iterator = opentxs::iterator::
            Bidirectional<const NotificationAccounts, const value_type>;

        virtual auto all() const noexcept
            -> UnallocatedSet<identifier::Account> = 0;
        /// Throws std::out_of_range for invalid position
        virtual auto at(const std::size_t position) const noexcept(false)
            -> const value_type& = 0;
        /// Throws std::out_of_range for invalid id
        virtual auto at(const identifier::Account& id) const noexcept(false)
            -> const value_type& = 0;
        virtual auto begin() const noexcept -> const_iterator = 0;
        virtual auto cbegin() const noexcept -> const_iterator = 0;
        virtual auto cend() const noexcept -> const_iterator = 0;
        virtual auto end() const noexcept -> const_iterator = 0;
        virtual auto size() const noexcept -> std::size_t = 0;
        virtual auto Type() const noexcept -> SubaccountType = 0;

        OPENTXS_NO_EXPORT virtual ~NotificationAccounts() = default;
    };
    struct OPENTXS_EXPORT PaymentCodeAccounts {
        using value_type = PaymentCode;
        using const_iterator = opentxs::iterator::
            Bidirectional<const PaymentCodeAccounts, const value_type>;

        virtual auto all() const noexcept
            -> UnallocatedSet<identifier::Account> = 0;
        /// Throws std::out_of_range for invalid position
        virtual auto at(const std::size_t position) const noexcept(false)
            -> const value_type& = 0;
        /// Throws std::out_of_range for invalid id
        virtual auto at(const identifier::Account& id) const noexcept(false)
            -> const value_type& = 0;
        virtual auto begin() const noexcept -> const_iterator = 0;
        virtual auto cbegin() const noexcept -> const_iterator = 0;
        virtual auto cend() const noexcept -> const_iterator = 0;
        virtual auto end() const noexcept -> const_iterator = 0;
        virtual auto size() const noexcept -> std::size_t = 0;
        virtual auto Type() const noexcept -> SubaccountType = 0;

        OPENTXS_NO_EXPORT virtual ~PaymentCodeAccounts() = default;
    };

    virtual auto AccountID() const noexcept -> const identifier::Account& = 0;
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    virtual auto GetHD() const noexcept -> const HDAccounts& = 0;
    /// Throws std::out_of_range if no keys are available
    virtual auto GetNextChangeKey(const PasswordPrompt& reason) const
        noexcept(false) -> const Element& = 0;
    /// Throws std::out_of_range if no keys are available
    virtual auto GetNextDepositKey(const PasswordPrompt& reason) const
        noexcept(false) -> const Element& = 0;
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
    virtual auto GetImported() const noexcept -> const ImportedAccounts& = 0;
    virtual auto GetNotification() const noexcept
        -> const NotificationAccounts& = 0;
    virtual auto GetPaymentCode() const noexcept
        -> const PaymentCodeAccounts& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> internal::Account& = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Parent() const noexcept -> const Wallet& = 0;
    virtual auto Subaccount(const identifier::Account& id) const noexcept(false)
        -> const crypto::Subaccount& = 0;

    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    OPENTXS_NO_EXPORT virtual ~Account() = default;

protected:
    Account() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
