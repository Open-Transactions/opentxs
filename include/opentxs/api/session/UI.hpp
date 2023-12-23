// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/interface/ui/Types.hpp"

class QAbstractItemModel;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class UI;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace identifier
{
class Account;
class Generic;
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace ui
{
class AccountActivityQt;
class AccountListQt;
class AccountSummaryQt;
class AccountTreeQt;
class ActivitySummaryQt;
class BlockchainAccountStatusQt;
class BlockchainSelectionQt;
class BlockchainStatisticsQt;
class ContactActivityQt;
class ContactActivityQtFilterable;
class ContactListQt;
class ContactQt;
class IdentityManagerQt;
class MessagableListQt;
class NymListQt;
class NymType;
class PayableListQt;
class ProfileQt;
class SeedListQt;
class SeedTreeQt;
class SeedValidator;
class UnitListQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class OPENTXS_EXPORT UI
{
public:
    /// Caller does not own this pointer
    virtual auto AccountActivityQt(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountActivityQt* = 0;
    /// Caller does not own this pointer
    virtual auto AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountListQt* = 0;
    /// Caller does not own this pointer
    virtual auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountSummaryQt* = 0;
    /// Caller does not own this pointer
    virtual auto AccountTreeQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountTreeQt* = 0;
    /// Caller does not own this pointer
    virtual auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ActivitySummaryQt* = 0;
    /// Caller does not own this pointer
    virtual auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel* = 0;
    /// Caller does not own this pointer
    virtual auto BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::BlockchainAccountStatusQt* = 0;
    virtual auto BlockchainIssuerID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Nym& = 0;
    virtual auto BlockchainNotaryID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Notary& = 0;
    /// Caller does not own this pointer
    virtual auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::BlockchainSelectionQt* = 0;
    virtual auto BlockchainStatisticsQt(const SimpleCallback updateCB = {})
        const noexcept -> opentxs::ui::BlockchainStatisticsQt* = 0;
    virtual auto BlockchainUnitID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::UnitDefinition& = 0;
    /// Caller does not own this pointer
    virtual auto ContactQt(
        const identifier::Generic& contactID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactQt* = 0;
    /// Caller does not own this pointer
    virtual auto ContactActivityQt(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactActivityQt* = 0;
    /// Caller does not own this pointer
    virtual auto ContactActivityQtFilterable(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactActivityQtFilterable* = 0;
    /// Caller does not own this pointer
    virtual auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactListQt* = 0;
    /// Caller does not own this pointer
    virtual auto IdentityManagerQt() const noexcept
        -> opentxs::ui::IdentityManagerQt* = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::UI& = 0;
    /// Caller does not own this pointer
    virtual auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::MessagableListQt* = 0;
    /// Caller does not own this pointer
    virtual auto NymListQt(const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::NymListQt* = 0;
    /// Caller does not own this pointer
    virtual auto NymType() const noexcept -> opentxs::ui::NymType* = 0;
    /// Caller does not own this pointer
    virtual auto PayableListQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::PayableListQt* = 0;
    /// Caller does not own this pointer
    virtual auto ProfileQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ProfileQt* = 0;
    /// Caller does not own this pointer
    virtual auto SeedListQt(const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::SeedListQt* = 0;
    /// Caller does not own this pointer
    /// Caller does not own this pointer
    virtual auto SeedTreeQt(const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::SeedTreeQt* = 0;
    /// Caller does not own this pointer
    virtual auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator* = 0;
    /// Caller does not own this pointer
    virtual auto UnitListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::UnitListQt* = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::UI& = 0;

    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;

    OPENTXS_NO_EXPORT virtual ~UI() = default;

protected:
    UI() = default;
};
}  // namespace opentxs::api::session
