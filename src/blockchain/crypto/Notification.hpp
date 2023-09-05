// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDPath.pb.h>
#include <cs_shared_guarded.h>
#include <shared_mutex>
#include <stdexcept>

#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Notification.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Notification final : public internal::Notification,
                           virtual public Subaccount
{
public:
    auto AllowedSubchains() const noexcept -> UnallocatedSet<Subchain> final;
    auto BalanceElement(const Subchain, const Bip32Index) const noexcept(false)
        -> const crypto::Element& final
    {
        throw std::out_of_range{
            "no balance elements present in notification subaccounts"};
    }
    auto LocalPaymentCode() const noexcept -> const opentxs::PaymentCode& final
    {
        return code_;
    }
    auto Path() const noexcept -> proto::HDPath final { return path_; }
    auto ScanProgress(Subchain type) const noexcept -> block::Position final;

    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    Notification(
        const api::Session& api,
        const crypto::Account& parent,
        const opentxs::PaymentCode& code,
        proto::HDPath&& path,
        identifier::Account& out) noexcept;
    Notification(const Notification&) = delete;
    Notification(Notification&&) = delete;
    auto operator=(const Notification&) -> Notification& = delete;
    auto operator=(Notification&&) -> Notification& = delete;

    ~Notification() final = default;

private:
    using ProgressMap = Map<Subchain, block::Position>;
    using GuardedProgressMap =
        libguarded::shared_guarded<ProgressMap, std::shared_mutex>;

    const opentxs::PaymentCode code_;
    const proto::HDPath path_;
    GuardedProgressMap progress_;

    static auto calculate_id(
        const api::Session& api,
        const blockchain::Type chain,
        const opentxs::PaymentCode& code) noexcept -> identifier::Account;

    auto account_already_exists(const rLock&) const noexcept -> bool final
    {
        return false;
    }
    auto check_activity(
        const rLock&,
        const UnallocatedVector<Activity>&,
        UnallocatedSet<identifier::Generic>&,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return false;
    }
    auto init() noexcept -> void final;
    auto save(const rLock&) const noexcept -> bool final { return false; }

    auto mutable_element(
        const rLock&,
        const Subchain,
        const Bip32Index) noexcept(false) -> crypto::Element& final
    {
        throw std::out_of_range{
            "no balance elements present in notification subaccounts"};
    }
};
}  // namespace opentxs::blockchain::crypto::implementation
