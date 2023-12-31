// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/verification/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PeerRequest.pb.h>
#include <opentxs/protobuf/VerificationRequest.pb.h>
#include <utility>

#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Claim.internal.hpp"

namespace opentxs::contract::peer::request::verification
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    const identity::wot::Claim& claim,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , claim_(claim, alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , claim_(api.Factory().Internal().Session().Claim(
          proto.verification().claim(),
          alloc))
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , VerificationPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , claim_(rhs.claim_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& verification = *out.mutable_verification();
    verification.set_version(Version());
    claim_.Internal().Serialize(*verification.mutable_claim());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::verification
