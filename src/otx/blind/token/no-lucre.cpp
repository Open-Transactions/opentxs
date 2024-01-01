// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/blind/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"
#include "opentxs/otx/blind/Token.hpp"

namespace opentxs::factory
{
auto TokenLucre(const otx::blind::Token&, otx::blind::internal::Purse&) noexcept
    -> otx::blind::Token
{
    return {};
}

auto TokenLucre(
    const api::Session&,
    otx::blind::internal::Purse&,
    const protobuf::Token&) noexcept -> otx::blind::Token
{
    return {};
}

auto TokenLucre(
    const api::Session&,
    const identity::Nym&,
    const otx::blind::Mint&,
    const otx::blind::Denomination,
    otx::blind::internal::Purse&,
    const opentxs::PasswordPrompt&) noexcept -> otx::blind::Token
{
    return {};
}
}  // namespace opentxs::factory
