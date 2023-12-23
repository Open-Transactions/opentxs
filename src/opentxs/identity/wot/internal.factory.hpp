// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
namespace wot
{
namespace internal
{
class Claim;
}  // namespace internal
}  // namespace wot
}  // namespace identity

namespace proto
{
class Claim;
class ContactItem;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Claim(
    const api::Session& api,
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    ReadView subtype,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    VersionNumber version,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*;
auto Claim(
    const api::Session& api,
    const proto::Claim& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*;
auto Claim(
    const api::Session& api,
    const identity::wot::Claimant& claimant,
    const identity::wot::claim::SectionType section,
    const proto::ContactItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::internal::Claim*;
}  // namespace opentxs::factory
