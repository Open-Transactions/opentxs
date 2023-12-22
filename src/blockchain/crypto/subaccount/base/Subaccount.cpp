// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto
{
Subaccount::Subaccount(std::shared_ptr<internal::Subaccount> imp) noexcept
    : imp_(std::move(imp))
{
    auto p = imp_.lock();
    assert_true(nullptr != p);
}

Subaccount::Subaccount(const Subaccount& rhs) noexcept = default;

Subaccount::Subaccount(Subaccount&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

auto Subaccount::AllowedSubchains() const noexcept -> Set<Subchain>
{
    if (auto p = imp_.lock(); p) {
        return p->AllowedSubchains();
    } else {
        return internal::Subaccount::Blank().AllowedSubchains();
    }
}

auto Subaccount::asDeterministic() const noexcept
    -> const crypto::Deterministic&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministicPublic();
    } else {
        return internal::Subaccount::Blank().asDeterministicPublic();
    }
}

auto Subaccount::asDeterministic() noexcept -> crypto::Deterministic&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministicPublic();
    } else {
        return internal::Subaccount::Blank().asDeterministicPublic();
    }
}

auto Subaccount::asImported() const noexcept -> const crypto::Imported&
{
    if (auto p = imp_.lock(); p) {
        return p->asImportedPublic();
    } else {
        return internal::Subaccount::Blank().asImportedPublic();
    }
}

auto Subaccount::asImported() noexcept -> crypto::Imported&
{
    if (auto p = imp_.lock(); p) {
        return p->asImportedPublic();
    } else {
        return internal::Subaccount::Blank().asImportedPublic();
    }
}

auto Subaccount::asNotification() const noexcept -> const crypto::Notification&
{
    if (auto p = imp_.lock(); p) {
        return p->asNotificationPublic();
    } else {
        return internal::Subaccount::Blank().asNotificationPublic();
    }
}

auto Subaccount::asNotification() noexcept -> crypto::Notification&
{
    if (auto p = imp_.lock(); p) {
        return p->asNotificationPublic();
    } else {
        return internal::Subaccount::Blank().asNotificationPublic();
    }
}

auto Subaccount::BalanceElement(const Subchain type, const Bip32Index index)
    const noexcept(false) -> const crypto::Element&
{
    if (auto p = imp_.lock(); p) {
        return p->BalanceElement(type, index);
    } else {
        return internal::Subaccount::Blank().BalanceElement(type, index);
    }
}

auto Subaccount::Blank() noexcept -> Subaccount&
{
    static auto blank = Subaccount{std::make_shared<internal::Subaccount>()};

    return blank;
}

auto Subaccount::Confirm(
    const Subchain type,
    const Bip32Index index,
    const block::TransactionHash& tx) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->Confirm(type, index, tx);
    } else {
        return internal::Subaccount::Blank().Confirm(type, index, tx);
    }
}

auto Subaccount::Describe() const noexcept -> std::string_view
{
    if (auto p = imp_.lock(); p) {
        return p->Describe();
    } else {
        return internal::Subaccount::Blank().Describe();
    }
}

auto Subaccount::ID() const noexcept -> const identifier::Account&
{
    if (auto p = imp_.lock(); p) {
        return p->ID();
    } else {
        return internal::Subaccount::Blank().ID();
    }
}

auto Subaccount::Internal() const noexcept -> const internal::Subaccount&
{
    return const_cast<Subaccount*>(this)->Internal();
}

auto Subaccount::Internal() noexcept -> internal::Subaccount&
{
    if (auto p = imp_.lock(); p) {

        return *p;
    } else {

        return internal::Subaccount::Blank();
    }
}

auto Subaccount::IsValid() const noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->IsValid();
    } else {
        return internal::Subaccount::Blank().IsValid();
    }
}

auto Subaccount::Parent() const noexcept -> const Account&
{
    if (auto p = imp_.lock(); p) {
        return p->Parent();
    } else {
        return internal::Subaccount::Blank().Parent();
    }
}

auto Subaccount::ScanProgress(Subchain subchain) const noexcept
    -> block::Position
{
    if (auto p = imp_.lock(); p) {
        return p->ScanProgress(subchain);
    } else {
        return internal::Subaccount::Blank().ScanProgress(subchain);
    }
}

auto Subaccount::SetScanProgress(
    const block::Position& progress,
    Subchain type) noexcept -> void
{
    if (auto p = imp_.lock(); p) {
        p->SetScanProgress(progress, type);
    } else {
        internal::Subaccount::Blank().SetScanProgress(progress, type);
    }
}

auto Subaccount::Type() const noexcept -> SubaccountType
{
    if (auto p = imp_.lock(); p) {
        return p->Type();
    } else {
        return internal::Subaccount::Blank().Type();
    }
}

auto Subaccount::Unconfirm(
    const Subchain type,
    const Bip32Index index,
    const block::TransactionHash& tx,
    const Time time) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->Unconfirm(type, index, tx, time);
    } else {
        return internal::Subaccount::Blank().Unconfirm(type, index, tx, time);
    }
}

Subaccount::~Subaccount() = default;
}  // namespace opentxs::blockchain::crypto
