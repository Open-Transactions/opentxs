// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/verification/Item.hpp"  // IWYU pragma: associated

#include <Signature.pb.h>
#include <VerificationItem.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/wot/Verification.hpp"
#include "internal/identity/wot/verification/Nym.hpp"
#include "internal/identity/wot/verification/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/internal.factory.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::VerificationItem(
    const identity::wot::verification::internal::Nym& parent,
    const identity::wot::ClaimID& claim,
    const identity::Nym& signer,
    const opentxs::PasswordPrompt& reason,
    identity::wot::verification::Type value,
    Time start,
    Time end,
    VersionNumber version,
    std::span<const identity::wot::VerificationID> superscedes)
    -> identity::wot::verification::internal::Item*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Item;

    try {

        return new ReturnType(
            parent,
            claim,
            signer,
            reason,
            value,
            start,
            end,
            version,
            superscedes);
    } catch (const std::exception& e) {
        LogError()()("Failed to construct verification item: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::VerificationItem(
    const identity::wot::verification::internal::Nym& parent,
    const proto::VerificationItem& serialized)
    -> identity::wot::verification::internal::Item*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Item;

    try {

        return new ReturnType(parent, serialized);
    } catch (const std::exception& e) {
        LogError()()("Failed to construct verification item: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::wot::verification
{
const VersionNumber Item::DefaultVersion{1};
}

namespace opentxs::identity::wot::verification::implementation
{
Item::Item(
    const internal::Nym& parent,
    const wot::ClaimID& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    Type value,
    Time start,
    Time end,
    VersionNumber version,
    std::span<const identity::wot::VerificationID> superscedes) noexcept(false)
    : version_(version)
    , claim_(claim)
    , value_(value)
    , start_(start)
    , end_(end)
    , id_(wot::internal::Verification::CalculateID(
          parent.API(),
          signer.ID(),
          claim_,
          version_,
          value_,
          start_,
          end_,
          superscedes,
          {}))
    , sig_(get_sig(
          signer,
          id_,
          claim_,
          version_,
          value_,
          start_,
          end_,
          superscedes,
          reason))
    , superscedes_([&] {
        auto out = decltype(superscedes_){};
        const auto& in = superscedes;
        out.reserve(in.size());
        std::ranges::copy(in, std::back_inserter(out));

        return out;
    }())
{
}

Item::Item(const internal::Nym& parent, const SerializedType& in) noexcept(
    false)
    : version_(in.version())
    , claim_(parent.API().Factory().Internal().Identifier(in.claim()))
    , value_(translate(in.kind()))
    , start_(convert_stime(in.start()))
    , end_(convert_stime(in.end()))
    , id_(parent.API().Factory().Internal().Identifier(in.id()))
    , sig_(in.sig())
    , superscedes_([&] {
        auto out = decltype(superscedes_){};
        const auto& proto = in.superscedes();
        const auto from_proto = [&](const auto& p) {
            return parent.API().Factory().Internal().Identifier(p);
        };
        out.reserve(proto.size());
        std::ranges::transform(proto, std::back_inserter(out), from_proto);

        return out;
    }())
{
    const auto calculated = wot::internal::Verification::CalculateID(
        parent.API(),
        parent.NymID(),
        claim_,
        version_,
        value_,
        start_,
        end_,
        superscedes_,
        {});

    if (id_ != calculated) { throw std::runtime_error("Invalid ID"); }
}

auto Item::get_sig(
    const identity::Nym& signer,
    const wot::VerificationID& id,
    const wot::ClaimID& claim,
    VersionNumber version,
    Type value,
    Time start,
    Time end,
    std::span<const identity::wot::VerificationID> superscedes,
    const PasswordPrompt& reason) noexcept(false) -> proto::Signature
{

    return wot::internal::Verification::Sign(
        signer, reason, id, claim, version, value, start, end, superscedes);
}

auto Item::Serialize(const api::Crypto&) const noexcept -> SerializedType
{
    auto output =
        sig_form(id_, claim_, version_, value_, start_, end_, superscedes_);
    output.mutable_sig()->CopyFrom(sig_);

    return output;
}

auto Item::sig_form(
    const wot::VerificationID& id,
    const wot::ClaimID& claim,
    VersionNumber version,
    Type value,
    Time start,
    Time end,
    std::span<const identity::wot::VerificationID> superscedes) noexcept
    -> SerializedType
{
    auto output = wot::internal::Verification::Serialize(
        claim, version, value, start, end, superscedes);
    id.Internal().Serialize(*output.mutable_id());

    return output;
}
}  // namespace opentxs::identity::wot::verification::implementation
