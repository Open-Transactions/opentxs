// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/bailment/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BailmentReply.pb.h>
#include <opentxs/protobuf/PeerReply.pb.h>
#include <utility>

#include "opentxs/identifier/Generic.hpp"

namespace opentxs::contract::peer::reply::bailment
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    std::string_view instructions,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , instructions_(instructions, alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , instructions_(proto.bailment().instructions())
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , instructions_(rhs.instructions_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& bailment = *out.mutable_bailment();
    bailment.set_version(Version());
    bailment.set_instructions(instructions_.data(), instructions_.size());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::bailment
