// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Deterministic.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"

namespace opentxs::blockchain::crypto
{
Deterministic::Deterministic(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Subaccount(std::move(imp))
{
}

Deterministic::Deterministic(const Deterministic& rhs) noexcept
    : Subaccount(rhs)
{
}

Deterministic::Deterministic(Deterministic&& rhs) noexcept
    : Subaccount(std::move(rhs))
{
}

auto Deterministic::asHD() const noexcept -> const crypto::HD&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asHDPublic();
    } else {
        return internal::Deterministic::Blank().asHDPublic();
    }
}

auto Deterministic::asHD() noexcept -> crypto::HD&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asHDPublic();
    } else {
        return internal::Deterministic::Blank().asHDPublic();
    }
}

auto Deterministic::asPaymentCode() const noexcept -> const crypto::PaymentCode&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCodePublic();
    } else {
        return internal::Deterministic::Blank().asPaymentCodePublic();
    }
}

auto Deterministic::asPaymentCode() noexcept -> crypto::PaymentCode&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().asPaymentCodePublic();
    } else {
        return internal::Deterministic::Blank().asPaymentCodePublic();
    }
}

auto Deterministic::Blank() noexcept -> Deterministic&
{
    static auto blank =
        Deterministic{std::make_shared<internal::Deterministic>()};

    return blank;
}

auto Deterministic::Floor(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Floor(type);
    } else {
        return internal::Deterministic::Blank().Floor(type);
    }
}

auto Deterministic::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().GenerateNext(type, reason);
    } else {
        return internal::Deterministic::Blank().GenerateNext(type, reason);
    }
}

auto Deterministic::Key(const Subchain type, const Bip32Index index)
    const noexcept -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Key(type, index);
    } else {
        return internal::Deterministic::Blank().Key(type, index);
    }
}

auto Deterministic::LastGenerated(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().LastGenerated(type);
    } else {
        return internal::Deterministic::Blank().LastGenerated(type);
    }
}

auto Deterministic::Lookahead() const noexcept -> std::size_t
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Lookahead();
    } else {
        return internal::Deterministic::Blank().Lookahead();
    }
}

auto Deterministic::PathRoot() const noexcept -> const opentxs::crypto::SeedID&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().PathRoot();
    } else {
        return internal::Deterministic::Blank().PathRoot();
    }
}

auto Deterministic::Reserve(
    const Subchain type,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Reserve(type, reason, label, time);
    } else {
        return internal::Deterministic::Blank().Reserve(
            type, reason, label, time);
    }
}

auto Deterministic::Reserve(
    const Subchain type,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Reserve(type, contact, reason, label, time);
    } else {
        return internal::Deterministic::Blank().Reserve(
            type, contact, reason, label, time);
    }
}

auto Deterministic::Reserve(
    const Subchain type,
    const std::size_t batch,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Reserve(type, batch, reason, label, time);
    } else {
        return internal::Deterministic::Blank().Reserve(
            type, batch, reason, label, time);
    }
}

auto Deterministic::Reserve(
    const Subchain type,
    const std::size_t batch,
    const identifier::Generic& contact,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().Reserve(
            type, batch, contact, reason, label, time);
    } else {
        return internal::Deterministic::Blank().Reserve(
            type, batch, contact, reason, label, time);
    }
}

auto Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::HD&
{
    if (auto p = imp_.lock(); p) {
        return p->asDeterministic().RootNode(reason);
    } else {
        return internal::Deterministic::Blank().RootNode(reason);
    }
}

Deterministic::~Deterministic() = default;
}  // namespace opentxs::blockchain::crypto
