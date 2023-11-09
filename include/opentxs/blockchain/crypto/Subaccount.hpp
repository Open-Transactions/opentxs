// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Subaccount;
}  // namespace internal

class Account;
class Deterministic;
class Element;
class Imported;
class Notification;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Account;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Subaccount
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Subaccount&;

    operator bool() const noexcept { return IsValid(); }

    auto asDeterministic() const noexcept -> const crypto::Deterministic&;
    auto asImported() const noexcept -> const crypto::Imported&;
    auto asNotification() const noexcept -> const crypto::Notification&;
    auto AllowedSubchains() const noexcept -> Set<Subchain>;
    /// Throws std::out_of_range for invalid index
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const crypto::Element&;
    auto Describe() const noexcept -> std::string_view;
    auto ID() const noexcept -> const identifier::Account&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Subaccount&;
    auto IsValid() const noexcept -> bool;
    auto Parent() const noexcept -> const Account&;
    auto ScanProgress(Subchain subchain) const noexcept -> block::Position;
    auto Type() const noexcept -> SubaccountType;

    auto asDeterministic() noexcept -> crypto::Deterministic&;
    auto asImported() noexcept -> crypto::Imported&;
    auto asNotification() noexcept -> crypto::Notification&;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Subaccount&;

    OPENTXS_NO_EXPORT Subaccount(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    Subaccount() = delete;
    Subaccount(const Subaccount& rhs) noexcept;
    Subaccount(Subaccount&& rhs) noexcept;
    auto operator=(const Subaccount&) -> Subaccount& = delete;
    auto operator=(Subaccount&&) -> Subaccount& = delete;

    virtual ~Subaccount();

protected:
    std::weak_ptr<internal::Subaccount> imp_;
};
}  // namespace opentxs::blockchain::crypto
