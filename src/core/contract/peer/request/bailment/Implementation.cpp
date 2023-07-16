// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/bailment/Implementation.hpp"  // IWYU pragma: associated

#include <Bailment.pb.h>
#include <PeerRequest.pb.h>
#include <utility>

#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs::contract::peer::request::bailment
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    identifier::Notary notary,
    identifier::UnitDefinition unit,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , notary_(std::move(notary), alloc)
    , unit_(std::move(unit), alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , notary_(
          api_.Factory().NotaryIDFromBase58(proto.bailment().serverid(), alloc))
    , unit_(api_.Factory().UnitIDFromBase58(proto.bailment().unitid(), alloc))
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , BailmentPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , notary_(rhs.notary_, alloc)
    , unit_(rhs.unit_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& bailment = *out.mutable_bailment();
    bailment.set_version(Version());
    bailment.set_unitid(unit_.asBase58(api_.Crypto()));
    bailment.set_serverid(notary_.asBase58(api_.Crypto()));

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::bailment