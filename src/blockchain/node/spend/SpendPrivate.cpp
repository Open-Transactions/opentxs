// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/spend/SpendPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/node/Funding.hpp"     // IWYU pragma: keep
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::blockchain::node
{
SpendPrivate::SpendPrivate() noexcept = default;

auto SpendPrivate::Funding() const noexcept -> node::Funding { return {}; }

auto SpendPrivate::ID() const noexcept -> const identifier::Generic&
{
    static const auto blank = identifier::Generic{};

    return blank;
}

auto SpendPrivate::Memo() const noexcept -> std::string_view { return {}; }

auto SpendPrivate::Notifications() const noexcept
    -> std::span<const PaymentCode>
{
    return {};
}

auto SpendPrivate::Notify(const PaymentCode&) noexcept -> bool { return {}; }

auto SpendPrivate::Notify(std::span<const PaymentCode>) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SendToAddress(std::string_view, const Amount&) noexcept
    -> bool
{
    return {};
}

auto SpendPrivate::SendToPaymentCode(const PaymentCode&, const Amount&) noexcept
    -> bool
{
    return {};
}

auto SpendPrivate::SetMemo(std::string_view) noexcept -> bool { return {}; }

auto SpendPrivate::SetSpendUnconfirmedChange(bool) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SetSpendUnconfirmedIncoming(bool) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SetSweepFromAccount(bool) noexcept -> bool { return {}; }

auto SpendPrivate::SetSweepFromKey(const crypto::Key&) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SetSweepFromSubaccount(const identifier::Account&) noexcept
    -> bool
{
    return {};
}

auto SpendPrivate::SetSweepToAddress(std::string_view) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SetUseEnhancedNotifications(bool) noexcept -> bool
{
    return {};
}

auto SpendPrivate::SpendUnconfirmedChange() const noexcept -> bool
{
    return {};
}

auto SpendPrivate::SpendUnconfirmedIncoming() const noexcept -> bool
{
    return {};
}

auto SpendPrivate::Spender() const noexcept -> const identifier::Nym&
{
    static const auto blank = identifier::Nym{};

    return blank;
}

auto SpendPrivate::SweepFromAccount() const noexcept -> bool { return {}; }

auto SpendPrivate::SweepFromKey() const noexcept -> const crypto::Key&
{
    static const auto blank = crypto::Key{};

    return blank;
}

auto SpendPrivate::SweepFromSubaccount() const noexcept
    -> const identifier::Account&
{
    static const auto blank = identifier::Account{};

    return blank;
}

auto SpendPrivate::SweepToAddress() const noexcept -> std::string_view
{
    return {};
}

auto SpendPrivate::UseEnhancedNotifications() const noexcept -> bool
{
    return {};
}

SpendPrivate::~SpendPrivate() = default;
}  // namespace opentxs::blockchain::node
