// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/blind/lucre/Mint.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/otx/blind/Factory.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "otx/blind/mint/Mint.hpp"

namespace opentxs::factory
{
auto MintLucre(const api::Session& api) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::Mint::Imp;

    return std::make_unique<ReturnType>(api).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary&,
    const identifier::UnitDefinition&) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::Mint::Imp;

    return std::make_unique<ReturnType>(api).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary&,
    const identifier::Nym&,
    const identifier::UnitDefinition&) noexcept -> otx::blind::Mint
{
    using ReturnType = otx::blind::Mint::Imp;

    return std::make_unique<ReturnType>(api).release();
}
}  // namespace opentxs::factory
