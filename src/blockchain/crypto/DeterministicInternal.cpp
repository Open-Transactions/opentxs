// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Deterministic.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>

#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Deterministic::Floor(const Subchain) const noexcept
    -> std::optional<Bip32Index>
{
    return {};
}

auto Deterministic::GenerateNext(const Subchain, const PasswordPrompt&)
    const noexcept -> std::optional<Bip32Index>
{
    return {};
}

auto Deterministic::Key(const Subchain, const Bip32Index) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    return opentxs::crypto::asymmetric::key::EllipticCurve::Blank();
}

auto Deterministic::LastGenerated(const Subchain) const noexcept
    -> std::optional<Bip32Index>
{
    return {};
}

auto Deterministic::Lookahead() const noexcept -> std::size_t { return {}; }

auto Deterministic::Path() const noexcept -> proto::HDPath { return {}; }

auto Deterministic::PathRoot() const noexcept -> const opentxs::crypto::SeedID&
{
    static const auto blank = opentxs::crypto::SeedID{};

    return blank;
}

auto Deterministic::Reserve(
    const Subchain,
    const identifier::Generic&,
    const PasswordPrompt&,
    const std::string_view,
    const Time) const noexcept -> std::optional<Bip32Index>
{
    return {};
}

auto Deterministic::Reserve(
    const Subchain,
    const PasswordPrompt&,
    const std::string_view,
    const Time) const noexcept -> std::optional<Bip32Index>
{
    return {};
}

auto Deterministic::Reserve(
    const Subchain type,
    const std::size_t,
    const identifier::Generic&,
    const PasswordPrompt&,
    const std::string_view,
    const Time) const noexcept -> Batch
{
    return {};
}

auto Deterministic::Reserve(
    const Subchain type,
    const std::size_t,
    const PasswordPrompt&,
    const std::string_view,
    const Time) const noexcept -> Batch
{
    return {};
}

auto Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::HD&
{
    return opentxs::crypto::asymmetric::key::HD::Blank();
}
}  // namespace opentxs::blockchain::crypto::internal
