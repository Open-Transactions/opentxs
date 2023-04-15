// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/verification/Item.hpp"  // IWYU pragma: associated

#include <Signature.pb.h>
#include <Verification.pb.h>
#include <stdexcept>

#include "2_Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/verification/Verification.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::VerificationItem(
    const identity::wot::verification::internal::Nym& parent,
    const identifier::Generic& claim,
    const identity::Nym& signer,
    const opentxs::PasswordPrompt& reason,
    const bool value,
    const Time start,
    const Time end,
    const VersionNumber version) -> identity::wot::verification::internal::Item*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Item;

    try {

        return new ReturnType(
            parent,
            claim,
            signer,
            reason,
            static_cast<ReturnType::Type>(value),
            start,
            end,
            version);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            "Failed to construct verification item: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::VerificationItem(
    const identity::wot::verification::internal::Nym& parent,
    const proto::Verification& serialized)
    -> identity::wot::verification::internal::Item*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Item;

    try {

        return new ReturnType(parent, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            "Failed to construct verification item: ")(e.what())
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
    const identifier::Generic& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const Type value,
    const Time start,
    const Time end,
    const VersionNumber version) noexcept(false)
    : version_(version)
    , claim_(claim)
    , value_(value)
    , valid_(Validity::Active)
    , start_(start)
    , end_(end)
    , id_(calculate_id(
          parent.API(),
          version_,
          claim_,
          value_,
          start_,
          end_,
          valid_,
          signer.ID()))
    , sig_(get_sig(
          signer,
          version_,
          id_,
          claim_,
          value_,
          start_,
          end_,
          valid_,
          reason))
{
}

Item::Item(const internal::Nym& parent, const SerializedType& in) noexcept(
    false)
    : version_(in.version())
    , claim_(parent.API().Factory().IdentifierFromBase58(in.claim()))
    , value_(static_cast<Type>(in.valid()))
    , valid_(static_cast<Validity>(in.retracted()))
    , start_(convert_stime(in.start()))
    , end_(convert_stime(in.end()))
    , id_(parent.API().Factory().IdentifierFromBase58(in.id()))
    , sig_(in.sig())
{
    const auto calculated = calculate_id(
        parent.API(),
        version_,
        claim_,
        value_,
        start_,
        end_,
        valid_,
        parent.NymID());

    if (id_ != calculated) { throw std::runtime_error("Invalid ID"); }
}

Item::operator SerializedType() const noexcept
{
    auto output = sig_form(version_, id_, claim_, value_, start_, end_, valid_);
    output.mutable_sig()->CopyFrom(sig_);

    return output;
}

auto Item::calculate_id(
    const api::Session& api,
    const VersionNumber version,
    const identifier::Generic& claim,
    const Type value,
    const Time start,
    const Time end,
    const Validity valid,
    const identifier::Nym& nym) noexcept(false) -> identifier::Generic
{
    const auto serialized = id_form(version, claim, value, start, end, valid);
    auto preimage = api.Factory().InternalSession().Data(serialized);
    preimage += nym;
    auto output = api.Factory().IdentifierFromPreimage(preimage.Bytes());

    if (output.empty()) { throw std::runtime_error("Unable to calculate ID"); }

    return output;
}

auto Item::get_sig(
    const identity::Nym& signer,
    const VersionNumber version,
    const identifier::Generic& id,
    const identifier::Generic& claim,
    const Type value,
    const Time start,
    const Time end,
    const Validity valid,
    const PasswordPrompt& reason) noexcept(false) -> proto::Signature
{
    auto serialized = sig_form(version, id, claim, value, start, end, valid);
    auto& sig = *serialized.mutable_sig();

    if (false == signer.Internal().Sign(
                     serialized,
                     crypto::SignatureRole::Claim,
                     *serialized.mutable_sig(),
                     reason)) {
        throw std::runtime_error("Unable to obtain signature");
    }

    return sig;
}

auto Item::id_form(
    const VersionNumber version,
    const identifier::Generic& claim,
    const Type value,
    const Time start,
    const Time end,
    const Validity valid) noexcept -> SerializedType
{
    auto output = SerializedType{};
    output.set_version(version);
    output.set_claim(claim.asBase58(Context().Crypto()));
    output.set_valid(static_cast<bool>(value));
    output.set_start(Clock::to_time_t(start));
    output.set_start(Clock::to_time_t(end));
    output.set_retracted(static_cast<bool>(valid));

    return output;
}

auto Item::sig_form(
    const VersionNumber version,
    const identifier::Generic& id,
    const identifier::Generic& claim,
    const Type value,
    const Time start,
    const Time end,
    const Validity valid) noexcept -> SerializedType
{
    auto output = id_form(version, claim, value, start, end, valid);
    output.set_id(id.asBase58(Context().Crypto()));

    return output;
}
}  // namespace opentxs::identity::wot::verification::implementation
