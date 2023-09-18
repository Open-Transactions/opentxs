// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identifier::Account
// IWYU pragma: no_include "opentxs/core/identifier/Account.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Generic.hpp"

#pragma once

#include "opentxs/blockchain/crypto/Subaccount.hpp"

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

class Account;
class Element;
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
struct Subaccount : virtual public crypto::Subaccount {
    auto AllowedSubchains() const noexcept -> UnallocatedSet<Subchain> override;
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const crypto::Element& override;
    auto Describe() const noexcept -> std::string_view override;
    auto ID() const noexcept -> const identifier::Account& override;
    auto Internal() const noexcept -> internal::Subaccount& final
    {
        return const_cast<Subaccount&>(*this);
    }
    auto IsValid() const noexcept -> bool override;
    auto Parent() const noexcept -> const crypto::Account& override;
    virtual auto PrivateKey(
        const implementation::Element& element,
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;
    auto ScanProgress(Subchain subchain) const noexcept
        -> block::Position override;
    auto Type() const noexcept -> SubaccountType override;

    virtual auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const block::TransactionHash& tx) noexcept -> bool;
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
};
}  // namespace opentxs::blockchain::crypto::internal
