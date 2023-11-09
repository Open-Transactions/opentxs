// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Account
// IWYU pragma: no_include "opentxs/core/identifier/Account.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Generic.hpp"

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace crypto
{
namespace implementation
{
class Element;
}  // namespace implementation

namespace internal
{
class Deterministic;
class Imported;
class Notification;
}  // namespace internal

class Account;
class Deterministic;
class Element;
class Imported;
class Notification;
class Subaccount;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
class Subaccount
{
public:
    static auto Blank() noexcept -> Subaccount&;

    virtual auto AllowedSubchains() const noexcept -> Set<Subchain>;
    virtual auto asDeterministic() const noexcept
        -> const internal::Deterministic&;
    virtual auto asDeterministicPublic() const noexcept
        -> const crypto::Deterministic&;
    virtual auto asNotification() const noexcept
        -> const internal::Notification&;
    virtual auto asNotificationPublic() const noexcept
        -> const crypto::Notification&;
    virtual auto asImported() const noexcept -> const internal::Imported&;
    virtual auto asImportedPublic() const noexcept -> const crypto::Imported&;
    virtual auto BalanceElement(const Subchain type, const Bip32Index index)
        const noexcept(false) -> const crypto::Element&;
    virtual auto DisplayName() const noexcept -> std::string_view;
    virtual auto DisplayType() const noexcept -> SubaccountType
    {
        return Type();
    }
    virtual auto Describe() const noexcept -> std::string_view;
    virtual auto ID() const noexcept -> const identifier::Account&;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Parent() const noexcept -> const crypto::Account&;
    virtual auto PrivateKey(
        const implementation::Element& element,
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;
    virtual auto Self() const noexcept -> const crypto::Subaccount&;
    virtual auto ScanProgress(Subchain subchain) const noexcept
        -> block::Position;
    virtual auto Source() const noexcept -> const identifier::Generic&;
    virtual auto SourceDescription() const noexcept -> std::string_view;
    virtual auto Type() const noexcept -> SubaccountType;

    virtual auto asDeterministic() noexcept -> internal::Deterministic&;
    virtual auto asDeterministicPublic() noexcept -> crypto::Deterministic&;
    virtual auto asNotification() noexcept -> internal::Notification&;
    virtual auto asNotificationPublic() noexcept -> crypto::Notification&;
    virtual auto asImported() noexcept -> internal::Imported&;
    virtual auto asImportedPublic() noexcept -> crypto::Imported&;
    virtual auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const block::TransactionHash& tx) noexcept -> bool;
    virtual auto InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void;
    virtual auto Self() noexcept -> crypto::Subaccount&;
    virtual auto SetContact(
        const Subchain type,
        const Bip32Index index,
        const identifier::Generic& id) noexcept(false) -> bool;
    virtual auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string_view label) noexcept(false) -> bool;
    virtual auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void;
    virtual auto UpdateElement(
        UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void;
    virtual auto Unconfirm(
        const Subchain type,
        const Bip32Index index,
        const block::TransactionHash& tx,
        const Time time) noexcept -> bool;
    virtual auto Unreserve(const Subchain type, const Bip32Index index) noexcept
        -> bool;

    Subaccount() = default;
    Subaccount(const Subaccount&) = delete;
    Subaccount(Subaccount&&) = delete;
    auto operator=(const Subaccount&) -> Subaccount& = delete;
    auto operator=(Subaccount&&) -> Subaccount& = delete;

    virtual ~Subaccount() = default;
};
}  // namespace opentxs::blockchain::crypto::internal
