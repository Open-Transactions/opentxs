// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/connection/Implementation.hpp"  // IWYU pragma: associated

#include <ConnectionInfo.pb.h>
#include <PeerRequest.pb.h>
#include <utility>

#include "opentxs/contract/peer/Types.internal.hpp"
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::request::connection
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    ConnectionInfoType kind,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , kind_(std::move(kind))
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , kind_(translate(proto.connectioninfo().type()))
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , kind_(rhs.kind_)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& connection = *out.mutable_connectioninfo();
    connection.set_version(Version());
    connection.set_type(translate(kind_));

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::connection
