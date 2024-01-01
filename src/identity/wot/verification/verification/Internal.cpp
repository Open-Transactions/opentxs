// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/identity/wot/Verification.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Signature.pb.h>
#include <opentxs/protobuf/Verification.pb.h>
#include <opentxs/protobuf/VerificationItem.pb.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/verification/Types.internal.hpp"

namespace opentxs::identity::wot::internal
{
auto Verification::CalculateID(
    const api::Session& api,
    const identifier::Nym& verifier,
    const wot::ClaimID& claim,
    VersionNumber version,
    verification::Type value,
    Time start,
    Time end,
    std::span<const VerificationID> superscedes,
    alloc::Strategy alloc) noexcept -> VerificationID
{
    const auto proto = [&] {
        auto out = protobuf::VerificationItem{};
        out.set_version(version);
        claim.Internal().Serialize(*out.mutable_claim());
        out.set_kind(translate(value));
        out.set_start(seconds_since_epoch(start).value());
        out.set_start(seconds_since_epoch(end).value());
        const auto serialize = [&](const auto& id) {
            id.Internal().Serialize(*out.add_superscedes());
        };
        std::ranges::for_each(superscedes, serialize);

        return out;
    }();

    return CalculateID(api, verifier, proto, alloc);
}

auto Verification::CalculateID(
    const api::Session& api,
    const identifier::Nym& verifier,
    const protobuf::VerificationItem& in,
    alloc::Strategy alloc) noexcept -> VerificationID
{
    const auto preimage = [&] {
        auto out = protobuf::Verification{};
        out.set_version(in.version());
        verifier.Internal().Serialize(*out.mutable_verifier());
        auto& item = *out.mutable_item();
        item.CopyFrom(in);
        item.clear_id();
        item.clear_sig();

        return out;
    }();

    return api.Factory().Internal().IdentifierFromPreimage(
        preimage, alloc.result_);
}

auto Verification::Serialize(
    const wot::ClaimID& claim,
    VersionNumber version,
    verification::Type value,
    Time start,
    Time end,
    std::span<const identity::wot::VerificationID> superscedes) noexcept
    -> protobuf::VerificationItem
{
    auto out = protobuf::VerificationItem{};
    out.set_version(version);
    claim.Internal().Serialize(*out.mutable_claim());
    out.set_kind(translate(value));
    out.set_start(seconds_since_epoch(start).value());
    out.set_end(seconds_since_epoch(end).value());
    const auto serialize = [&](const auto& id) {
        id.Internal().Serialize(*out.add_superscedes());
    };
    std::ranges::for_each(superscedes, serialize);

    return out;
}

auto Verification::Serialize(
    const identifier::Nym& verifier,
    const wot::ClaimID& claim,
    VersionNumber version,
    verification::Type value,
    Time start,
    Time end,
    std::span<const identity::wot::VerificationID> superscedes) noexcept
    -> protobuf::Verification
{
    auto out = protobuf::Verification{};
    out.set_version(version);
    verifier.Internal().Serialize(*out.mutable_verifier());
    out.mutable_item()->CopyFrom(
        Serialize(claim, version, value, start, end, superscedes));

    return out;
}

auto Verification::Serialize(protobuf::Verification&) const noexcept -> void {}

auto Verification::Sign(
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const wot::VerificationID& id,
    const wot::ClaimID& claim,
    VersionNumber version,
    verification::Type value,
    Time start,
    Time end,
    std::span<const identity::wot::VerificationID> superscedes) noexcept(false)
    -> protobuf::Signature
{
    auto proto =
        Serialize(signer.ID(), claim, version, value, start, end, superscedes);
    auto& item = *proto.mutable_item();
    id.Internal().Serialize(*item.mutable_id());
    auto& sig = *item.mutable_sig();

    if (false == signer.Internal().Sign(
                     proto, crypto::SignatureRole::Claim, sig, reason)) {
        throw std::runtime_error("Unable to obtain signature");
    }

    return sig;
}
}  // namespace opentxs::identity::wot::internal
