// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/internal.factory.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "opentxs/core/identifier/Nym.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/ItemPrivate.hpp"
#include "opentxs/identity/wot/internal.factory.hpp"

namespace opentxs::factory
{
auto ContactItem(
    const api::Session& api,
    const proto::ContactItem& proto,
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    alloc::Strategy alloc) noexcept -> identity::wot::claim::internal::Item*
{
    using ReturnType = identity::wot::claim::ItemPrivate;

    return pmr::construct<ReturnType>(
        alloc.result_, Claim(api, claimant, section, proto, alloc.result_));
}

auto ContactItem(
    const identity::wot::Claim& claim,
    alloc::Strategy alloc) noexcept -> identity::wot::claim::internal::Item*
{
    using ReturnType = identity::wot::claim::ItemPrivate;

    return pmr::construct<ReturnType>(alloc.result_, claim);
}

auto ContactItem(identity::wot::Claim&& claim, alloc::Strategy alloc) noexcept
    -> opentxs::identity::wot::claim::internal::Item*
{
    using ReturnType = identity::wot::claim::ItemPrivate;

    return pmr::construct<ReturnType>(alloc.result_, std::move(claim));
}
}  // namespace opentxs::factory
