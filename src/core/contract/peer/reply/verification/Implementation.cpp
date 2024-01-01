// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/verification/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PeerReply.pb.h>
#include <opentxs/protobuf/VerificationReply.pb.h>
#include <utility>

#include "internal/identity/wot/Verification.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/Verification.hpp"

namespace opentxs::contract::peer::reply::verification
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    std::optional<identity::wot::Verification> response,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , verification_(std::move(response))
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , verification_([&]() -> decltype(verification_) {
        if (proto.verification().has_response()) {

            return api.Factory().Internal().Session().Verification(
                proto.verification().response(), alloc);
        } else {

            return std::nullopt;
        }
    }())
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , verification_([&]() -> decltype(verification_) {
        if (rhs.verification_.has_value()) {

            return identity::wot::Verification{*rhs.verification_, alloc};
        } else {

            return std::nullopt;
        }
    }())
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& verification = *out.mutable_verification();
    verification.set_version(Version());

    if (verification_.has_value()) {
        verification_->Internal().Serialize(*verification.mutable_response());
    }

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::verification
