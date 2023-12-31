// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"

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
namespace claim
{
namespace internal
{
class Item;
}  // namespace internal
}  // namespace claim

class Claim;
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class ContactItem;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto ContactItem(
    const api::Session& api,
    const protobuf::ContactItem& proto,
    const identity::wot::Claimant& claimant,
    identity::wot::claim::SectionType section,
    alloc::Strategy alloc) noexcept -> identity::wot::claim::internal::Item*;
auto ContactItem(
    const identity::wot::Claim& claim,
    alloc::Strategy alloc) noexcept -> identity::wot::claim::internal::Item*;
auto ContactItem(identity::wot::Claim&& claim, alloc::Strategy alloc) noexcept
    -> identity::wot::claim::internal::Item*;
}  // namespace opentxs::factory
