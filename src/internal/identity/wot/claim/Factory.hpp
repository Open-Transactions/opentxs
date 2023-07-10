// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
class ClaimPrivate;
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
    const identifier::Nym& claimant,
    identity::wot::claim::SectionType section,
    identity::wot::claim::ClaimType type,
    ReadView value,
    std::span<const identity::wot::claim::Attribute> attributes,
    Time start,
    Time stop,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*;
auto Claim(
    const api::Session& api,
    const proto::Claim& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*;
auto Claim(
    const api::Session& api,
    const identifier::Nym& claimant,
    const identity::wot::claim::SectionType section,
    const proto::ContactItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::ClaimPrivate*;
}  // namespace opentxs::factory
