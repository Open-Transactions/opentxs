// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/outbailment/Implementation.hpp"  // IWYU pragma: associated

#include <OutBailmentReply.pb.h>
#include <PeerReply.pb.h>
#include <utility>

#include "opentxs/identifier/Generic.hpp"

namespace opentxs::contract::peer::reply::outbailment
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    std::string_view description,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , description_(description, alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , description_(proto.outbailment().instructions(), alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , description_(rhs.description_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& outbailment = *out.mutable_outbailment();
    outbailment.set_version(Version());
    outbailment.set_instructions(description_.data(), description_.size());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::outbailment
