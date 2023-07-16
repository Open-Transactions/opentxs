// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/verification/verification/Implementation.hpp"  // IWYU pragma: associated

#include <Signature.pb.h>
#include <VerificationItem.pb.h>
#include <algorithm>
#include <exception>
#include <iterator>
#include <memory>
#include <utility>

#include "identity/wot/verification/verification/VerificationPrivate.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/identity/wot/Verification.hpp"
#include "internal/identity/wot/verification/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Verification.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::identity::wot::verification::implementation
{
Verification::Verification(
    const api::Session& api,
    Nym_p verifier,
    ClaimID claim,
    verification::Type value,
    Time start,
    Time stop,
    std::span<const VerificationID> superscedes,
    allocator_type alloc) noexcept(false)
    : VerificationPrivate(alloc)
    , Signable<wot::Verification::identifier_type>(
          api,
          verifier,
          default_version_,
          {},
          {})
    , claim_(std::move(claim), alloc)
    , value_(value)
    , start_(start)
    , stop_(stop)
    , superscedes_(copy_construct(superscedes, alloc))
{
    first_time_init();
}

Verification::Verification(
    const api::Session& api,
    const proto::VerificationItem& proto,
    Nym_p verifier,
    allocator_type alloc) noexcept(false)
    : VerificationPrivate(alloc)
    , Signable<wot::Verification::identifier_type>(
          api,
          verifier,
          default_version_,
          {},
          {},
          api.Factory().Internal().Identifier(proto.id()),
          [&] {
              auto out = Signatures{};
              out.emplace_back(std::make_shared<proto::Signature>(proto.sig()));

              return out;
          }())
    , claim_(api.Factory().Internal().Identifier(proto.claim()), alloc)
    , value_(translate(proto.kind()))
    , start_(convert_stime(proto.start()))
    , stop_(convert_stime(proto.end()))
    , superscedes_([&] {
        auto out = decltype(superscedes_){};
        const auto& ids = proto.superscedes();
        const auto from_proto = [&](const auto& p) {
            return api.Factory().Internal().Identifier(p);
        };
        out.reserve(ids.size());
        std::transform(
            ids.begin(), ids.end(), std::back_inserter(out), from_proto);

        return out;
    }())
{
    init_serialized();
}

Verification::Verification(
    const Verification& rhs,
    allocator_type alloc) noexcept
    : VerificationPrivate(rhs, alloc)
    , Signable<wot::Verification::identifier_type>(rhs)
    , claim_(rhs.claim_, alloc)
    , value_(rhs.value_)
    , start_(rhs.start_)
    , stop_(rhs.stop_)
    , superscedes_(rhs.superscedes_, alloc)
{
}

auto Verification::calculate_id() const -> identifier_type
{
    return wot::internal::Verification::CalculateID(
        api_,
        Signer()->ID(),
        claim_,
        Version(),
        value_,
        start_,
        stop_,
        superscedes_,
        {});
}

auto Verification::final_form() const noexcept -> proto::Verification
{
    auto out = signing_form();
    auto& item = *out.mutable_item();
    item.mutable_sig()->CopyFrom(*signatures()[0]);

    return out;
}

auto Verification::Finish(const PasswordPrompt& reason) noexcept -> bool
{
    if (update_signature(reason)) {

        return validate();
    } else {

        return false;
    }
}

auto Verification::Serialize(Writer&& out) const noexcept -> bool
{
    return proto::write(final_form(), std::move(out));
}

auto Verification::Serialize(proto::Verification& out) const noexcept -> void
{
    out = final_form();
}

auto Verification::signing_form() const noexcept -> proto::Verification
{
    auto out = wot::internal::Verification::Serialize(
        Signer()->ID(), claim_, Version(), value_, start_, stop_, superscedes_);
    auto& item = *out.mutable_item();
    ID().Internal().Serialize(*item.mutable_id());

    return out;
}

auto Verification::update_signature(const PasswordPrompt& reason) -> bool
{
    if (false == Signable<wot::Verification::identifier_type>::update_signature(
                     reason)) {
        return false;
    }

    try {
        add_signatures([&, this] {
            auto out = Signatures{};
            out.emplace_back(std::make_shared<proto::Signature>(
                wot::internal::Verification::Sign(
                    *Signer(),
                    reason,
                    ID(),
                    claim_,
                    Version(),
                    value_,
                    start_,
                    stop_,
                    superscedes_)));

            return out;
        }());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Verification::validate() const -> bool
{
    if (calculate_id() != ID()) {
        LogError()(OT_PRETTY_CLASS())("id mismatch").Flush();

        return false;
    }

    if (false == proto::Validate(final_form(), VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("invalid syntax").Flush();

        return false;
    }

    const auto sigs = signatures();

    if (sigs.empty()) {
        LogError()(OT_PRETTY_CLASS())("missing signature").Flush();

        return false;
    }

    const auto& sig = sigs[0];

    if (false == sig.operator bool()) {
        LogError()(OT_PRETTY_CLASS())("null signature").Flush();

        return false;
    }

    if (false == verify_signature(*sig)) {
        LogError()(OT_PRETTY_CLASS())("invalid signature").Flush();

        return false;
    }

    return true;
}

auto Verification::verify_signature(const proto::Signature& signature) const
    -> bool
{
    if (false == Signable<wot::Verification::identifier_type>::verify_signature(
                     signature)) {

        return false;
    }

    auto proto = signing_form();
    auto& item = *proto.mutable_item();
    auto& sig = *item.mutable_sig();
    sig.CopyFrom(signature);

    return Signer()->Internal().Verify(proto, sig);
}
}  // namespace opentxs::identity::wot::verification::implementation