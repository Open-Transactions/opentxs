// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <opentxs/protobuf/HDPath.pb.h>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <stdexcept>

#include "blockchain/crypto/subaccount/base/Imp.hpp"
#include "internal/blockchain/crypto/Notification.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"
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
namespace internal
{
class Subaccount;
}  // namespace internal

class Account;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class NotificationPrivate final : public internal::Notification,
                                  public SubaccountPrivate
{
public:
    auto AllowedSubchains() const noexcept -> Set<Subchain> final;
    auto asNotification() const noexcept -> const internal::Notification& final
    {
        return *this;
    }
    auto asNotificationPublic() const noexcept
        -> const crypto::Notification& final
    {
        return const_cast<NotificationPrivate*>(this)->asNotificationPublic();
    }
    auto BalanceElement(const Subchain, const Bip32Index) const noexcept(false)
        -> const crypto::Element& final
    {
        throw std::out_of_range{
            "no balance elements present in notification subaccounts"};
    }
    auto DisplayType() const noexcept -> SubaccountType final;
    auto LocalPaymentCode() const noexcept -> const opentxs::PaymentCode& final
    {
        return code_;
    }
    auto Path() const noexcept -> protobuf::HDPath final { return path_; }
    auto ScanProgress(Subchain type) const noexcept -> block::Position final;
    auto Self() const noexcept -> const crypto::Subaccount& final
    {
        return asNotificationPublic();
    }

    auto asNotification() noexcept -> internal::Notification& final
    {
        return *this;
    }
    auto asNotificationPublic() noexcept -> crypto::Notification& final
    {
        return *self_;
    }
    auto InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void final;
    auto Self() noexcept -> crypto::Subaccount& final
    {
        return asNotificationPublic();
    }
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    NotificationPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const opentxs::PaymentCode& code,
        protobuf::HDPath&& path) noexcept(false);
    NotificationPrivate(const NotificationPrivate&) = delete;
    NotificationPrivate(NotificationPrivate&&) = delete;
    auto operator=(const NotificationPrivate&) -> NotificationPrivate& = delete;
    auto operator=(NotificationPrivate&&) -> NotificationPrivate& = delete;

    ~NotificationPrivate() final = default;

private:
    using ProgressMap = Map<Subchain, block::Position>;
    using GuardedProgressMap =
        libguarded::shared_guarded<ProgressMap, std::shared_mutex>;
    using Me = std::optional<crypto::Notification>;

    const opentxs::PaymentCode code_;
    const protobuf::HDPath path_;
    GuardedProgressMap progress_;
    mutable Me self_;

    auto account_already_exists(const rLock&) const noexcept -> bool final
    {
        return false;
    }
    auto init(bool existing) noexcept(false) -> void final;
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
}  // namespace opentxs::blockchain::crypto
