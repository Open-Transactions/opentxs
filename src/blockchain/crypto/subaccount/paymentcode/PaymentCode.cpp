// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/PaymentCode.hpp"  // IWYU pragma: associated

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/PaymentCode.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"

namespace opentxs::blockchain::crypto
{
PaymentCode::PaymentCode(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Deterministic(std::move(imp))
{
}

PaymentCode::PaymentCode(const PaymentCode& rhs) noexcept = default;

PaymentCode::PaymentCode(PaymentCode&& rhs) noexcept
    : Deterministic(std::move(rhs))
{
}

auto PaymentCode::Blank() noexcept -> PaymentCode&
{
    static auto blank = PaymentCode{std::make_shared<internal::PaymentCode>()};

    return blank;
}

auto PaymentCode::IncomingNotificationCount() const noexcept -> std::size_t
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCode().IncomingNotificationCount();
    } else {
        return internal::PaymentCode::Blank().IncomingNotificationCount();
    }
}

auto PaymentCode::Local() const noexcept -> const opentxs::PaymentCode&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCode().Local();
    } else {
        return internal::PaymentCode::Blank().Local();
    }
}

auto PaymentCode::NotificationCount() const noexcept
    -> std::pair<std::size_t, std::size_t>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCode().NotificationCount();
    } else {
        return internal::PaymentCode::Blank().NotificationCount();
    }
}

auto PaymentCode::OutgoingNotificationCount() const noexcept -> std::size_t
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCode().OutgoingNotificationCount();
    } else {
        return internal::PaymentCode::Blank().OutgoingNotificationCount();
    }
}

auto PaymentCode::Remote() const noexcept -> const opentxs::PaymentCode&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCode().Remote();
    } else {
        return internal::PaymentCode::Blank().Remote();
    }
}

PaymentCode::~PaymentCode() = default;
}  // namespace opentxs::blockchain::crypto
