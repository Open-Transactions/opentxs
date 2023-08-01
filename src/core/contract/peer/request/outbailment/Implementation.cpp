// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/outbailment/Implementation.hpp"  // IWYU pragma: associated

#include <OutBailment.pb.h>
#include <PeerRequest.pb.h>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::contract::peer::request::outbailment
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    identifier::Notary notary,
    identifier::UnitDefinition unit,
    std::string_view instructions,
    opentxs::Amount amount,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , notary_(std::move(notary), alloc)
    , unit_(std::move(unit), alloc)
    , instructions_(instructions, alloc)
    , amount_(std::move(amount))
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , notary_(api_.Factory().Internal().NotaryID(
          proto.outbailment().serverid(),
          alloc))
    , unit_(
          api_.Factory().Internal().UnitID(proto.outbailment().unitid(), alloc))
    , instructions_(proto.outbailment().instructions(), alloc)
    , amount_(factory::Amount(proto.outbailment().amount()))
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , OutbailmentPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , notary_(rhs.notary_, alloc)
    , unit_(rhs.unit_, alloc)
    , instructions_(rhs.instructions_, alloc)
    , amount_(rhs.amount_)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& outbailment = *out.mutable_outbailment();
    outbailment.set_version(Version());
    unit_.Internal().Serialize(*outbailment.mutable_unitid());
    notary_.Internal().Serialize(*outbailment.mutable_serverid());
    amount_.Serialize(writer(outbailment.mutable_amount()));
    outbailment.set_instructions(instructions_.data(), instructions_.size());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::outbailment
