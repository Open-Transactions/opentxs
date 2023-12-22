// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Notification.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Notification.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"

namespace opentxs::blockchain::crypto
{
Notification::Notification(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Subaccount(std::move(imp))
{
}

Notification::Notification(const Notification& rhs) noexcept = default;

Notification::Notification(Notification&& rhs) noexcept
    : Subaccount(std::move(rhs))
{
}

auto Notification::Blank() noexcept -> Notification&
{
    static auto blank =
        Notification{std::make_shared<internal::Notification>()};

    return blank;
}

auto Notification::LocalPaymentCode() const noexcept
    -> const opentxs::PaymentCode&
{
    if (auto p = imp_.lock(); p) {
        return p->asNotification().LocalPaymentCode();
    } else {
        return internal::Notification::Blank().LocalPaymentCode();
    }
}

Notification::~Notification() = default;
}  // namespace opentxs::blockchain::crypto
