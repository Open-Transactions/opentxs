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
#include "opentxs/util/Numbers.hpp"

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
class Nym;
}  // namespace identity

namespace protobuf
{
class Signature;
class Verification;
class VerificationItem;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::internal
{
class Verification
{
public:
    static auto CalculateID(
        const api::Session& api,
        const identifier::Nym& verifier,
        const wot::ClaimID& claim,
        VersionNumber version,
        verification::Type value,
        Time start,
        Time end,
        std::span<const VerificationID> superscedes,
        alloc::Strategy alloc) noexcept -> VerificationID;
    static auto CalculateID(
        const api::Session& api,
        const identifier::Nym& verifier,
        const protobuf::VerificationItem& serialized,
        alloc::Strategy alloc) noexcept -> VerificationID;
    static auto Serialize(
        const wot::ClaimID& claim,
        VersionNumber version,
        verification::Type value,
        Time start,
        Time end,
        std::span<const identity::wot::VerificationID> superscedes) noexcept
        -> protobuf::VerificationItem;
    static auto Serialize(
        const identifier::Nym& verifier,
        const wot::ClaimID& claim,
        VersionNumber version,
        verification::Type value,
        Time start,
        Time end,
        std::span<const identity::wot::VerificationID> superscedes) noexcept
        -> protobuf::Verification;
    static auto Sign(
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const wot::VerificationID& id,
        const wot::ClaimID& claim,
        VersionNumber version,
        verification::Type value,
        Time start,
        Time end,
        std::span<const identity::wot::VerificationID>
            superscedes) noexcept(false) -> protobuf::Signature;

    virtual auto Serialize(protobuf::Verification& out) const noexcept -> void;

    virtual ~Verification() = default;
};
}  // namespace opentxs::identity::wot::internal
