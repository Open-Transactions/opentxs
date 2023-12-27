// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/Time.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Allocator.hpp"

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
class VerificationPrivate;
}  // namespace wot
}  // namespace identity

namespace proto
{
class Verification;
class VerificationItem;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto Verification(
    const api::Session& api,
    const identifier::Nym& verifier,
    const opentxs::PasswordPrompt& reason,
    identity::wot::ClaimID claim,
    identity::wot::verification::Type value,
    Time start,
    Time stop,
    std::span<const identity::wot::VerificationID> superscedes,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*;
auto Verification(
    const api::Session& api,
    const proto::Verification& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*;
auto Verification(
    const api::Session& api,
    const identifier::Nym& verifier,
    const proto::VerificationItem& proto,
    alloc::Strategy alloc) noexcept -> identity::wot::VerificationPrivate*;
}  // namespace opentxs::factory
