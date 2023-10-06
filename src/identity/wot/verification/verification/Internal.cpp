// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/identity/wot/Verification.hpp"  // IWYU pragma: associated

#include <Signature.pb.h>
#include <Verification.pb.h>
#include <VerificationItem.pb.h>
#include <algorithm>
#include <functional>
#include <stdexcept>

#include "internal/api/FactoryAPI.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/verification/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"

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
        auto out = proto::VerificationItem{};
        out.set_version(version);
        claim.Internal().Serialize(*out.mutable_claim());
        out.set_kind(translate(value));
        out.set_start(Clock::to_time_t(start));
        out.set_start(Clock::to_time_t(end));
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
    const proto::VerificationItem& in,
    alloc::Strategy alloc) noexcept -> VerificationID
{
    const auto preimage = [&] {
        auto out = proto::Verification{};
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
    -> proto::VerificationItem
{
    auto out = proto::VerificationItem{};
    out.set_version(version);
    claim.Internal().Serialize(*out.mutable_claim());
    out.set_kind(translate(value));
    out.set_start(Clock::to_time_t(start));
    out.set_end(Clock::to_time_t(end));
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
    -> proto::Verification
{
    auto out = proto::Verification{};
    out.set_version(version);
    verifier.Internal().Serialize(*out.mutable_verifier());
    out.mutable_item()->CopyFrom(
        Serialize(claim, version, value, start, end, superscedes));

    return out;
}

auto Verification::Serialize(proto::Verification&) const noexcept -> void {}

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
    -> proto::Signature
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
