// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/sodium/Sodium.hpp"  // IWYU pragma: associated

namespace opentxs::crypto::implementation
{
auto Sodium::PubkeyAdd(const ReadView, const ReadView, Writer&&) const noexcept
    -> bool
{
    return {};
}

auto Sodium::RandomKeypair(
    Writer&&,
    Writer&&,
    const opentxs::crypto::asymmetric::Role,
    const Parameters&,
    Writer&&) const noexcept -> bool
{
    return {};
}

auto Sodium::ScalarAdd(const ReadView, const ReadView, Writer&&) const noexcept
    -> bool
{
    return {};
}

auto Sodium::ScalarMultiplyBase(const ReadView, Writer&&) const noexcept -> bool
{
    return {};
}

auto Sodium::SharedSecret(
    const ReadView,
    const ReadView,
    const SecretStyle,
    Secret&) const noexcept -> bool
{
    return {};
}

auto Sodium::Sign(
    const ReadView,
    const ReadView,
    const crypto::HashType,
    Writer&&) const -> bool
{
    return {};
}

auto Sodium::Verify(
    const ReadView,
    const ReadView,
    const ReadView,
    const crypto::HashType) const -> bool
{
    return {};
}
}  // namespace opentxs::crypto::implementation
