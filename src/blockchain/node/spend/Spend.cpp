// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/node/Spend.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "blockchain/node/spend/SpendPrivate.hpp"
#include "internal/util/LogMacros.hpp"

namespace opentxs::blockchain::node
{
Spend::Spend(SpendPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Spend::Spend() noexcept
    : Spend(std::make_unique<SpendPrivate>().release())
{
}

Spend::Spend(Spend&& rhs) noexcept
    : Spend(std::exchange(rhs.imp_, nullptr))
{
}

auto Spend::Funding() const noexcept -> node::Funding
{
    return imp_->Funding();
}

auto Spend::ID() const noexcept -> const identifier::Generic&
{
    return imp_->ID();
}

auto Spend::Internal() const noexcept -> const internal::Spend&
{
    return *imp_;
}

auto Spend::Internal() noexcept -> internal::Spend& { return *imp_; }

auto Spend::Memo() const noexcept -> std::string_view { return imp_->Memo(); }

auto Spend::Notifications() const noexcept -> std::span<const PaymentCode>
{
    return imp_->Notifications();
}

auto Spend::Notify(const PaymentCode& recipient) noexcept -> bool
{
    return imp_->Notify(recipient);
}

auto Spend::Notify(std::span<const PaymentCode> recipients) noexcept -> bool
{
    return imp_->Notify(recipients);
}

auto Spend::SendToAddress(
    std::string_view address,
    const Amount& amount) noexcept -> bool
{
    return imp_->SendToAddress(address, amount);
}

auto Spend::SendToPaymentCode(
    const PaymentCode& recipient,
    const Amount& amount) noexcept -> bool
{
    return imp_->SendToPaymentCode(recipient, amount);
}

auto Spend::SetMemo(std::string_view memo) noexcept -> bool
{
    return imp_->SetMemo(memo);
}

auto Spend::SetSpendUnconfirmedChange(bool value) noexcept -> bool
{
    return imp_->SetSpendUnconfirmedChange(value);
}

auto Spend::SetSpendUnconfirmedIncoming(bool value) noexcept -> bool
{
    return imp_->SetSpendUnconfirmedIncoming(value);
}

auto Spend::SetSweepFromAccount(bool value) noexcept -> bool
{
    return imp_->SetSweepFromAccount(value);
}

auto Spend::SetSweepFromKey(const crypto::Key& key) noexcept -> bool
{
    return imp_->SetSweepFromKey(key);
}

auto Spend::SetSweepFromSubaccount(
    const identifier::Account& aubaccount) noexcept -> bool
{
    return imp_->SetSweepFromSubaccount(aubaccount);
}

auto Spend::SetSweepToAddress(std::string_view address) noexcept -> bool
{
    return imp_->SetSweepToAddress(address);
}

auto Spend::SetUseEnhancedNotifications(bool value) noexcept -> bool
{
    return imp_->SetUseEnhancedNotifications(value);
}

auto Spend::SpendUnconfirmedChange() const noexcept -> bool
{
    return imp_->SpendUnconfirmedChange();
}

auto Spend::SpendUnconfirmedIncoming() const noexcept -> bool
{
    return imp_->SpendUnconfirmedIncoming();
}

auto Spend::Spender() const noexcept -> const identifier::Nym&
{
    return imp_->Spender();
}

auto Spend::SweepFromAccount() const noexcept -> bool
{
    return imp_->SweepFromAccount();
}

auto Spend::SweepFromKey() const noexcept -> const crypto::Key&
{
    return imp_->SweepFromKey();
}

auto Spend::SweepFromSubaccount() const noexcept -> const identifier::Account&
{
    return imp_->SweepFromSubaccount();
}

auto Spend::SweepToAddress() const noexcept -> std::string_view
{
    return imp_->SweepToAddress();
}

auto Spend::UseEnhancedNotifications() const noexcept -> bool
{
    return imp_->UseEnhancedNotifications();
}

Spend::~Spend()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node
