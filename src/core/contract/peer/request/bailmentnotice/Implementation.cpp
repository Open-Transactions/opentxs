// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/bailmentnotice/Implementation.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <PendingBailment.pb.h>
#include <utility>

#include "internal/core/Factory.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::contract::peer::request::bailmentnotice
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    identifier::Notary notary,
    identifier::UnitDefinition unit,
    peer::Request::identifier_type ref,
    std::string_view description,
    opentxs::Amount amount,
    allocator_type alloc) noexcept(false)
    : RequestPrivate(alloc)
    , BailmentNoticePrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          alloc)
    , notary_(std::move(notary), alloc)
    , unit_(std::move(unit), alloc)
    , in_reference_to_(std::move(ref))
    , description_(description, alloc)
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
    , BailmentNoticePrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , notary_(api_.Factory().Internal().NotaryID(
          proto.pendingbailment().serverid(),
          alloc))
    , unit_(api_.Factory().Internal().UnitID(
          proto.pendingbailment().unitid(),
          alloc))
    , in_reference_to_(api_.Factory().Internal().Identifier(
          proto.pendingbailment().requestid(),
          alloc))
    , description_(proto.pendingbailment().txid(), alloc)
    , amount_(factory::Amount(proto.pendingbailment().amount()))
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(alloc)
    , BailmentNoticePrivate(alloc)
    , base::Implementation(rhs, alloc)
    , notary_(rhs.notary_, alloc)
    , unit_(rhs.unit_, alloc)
    , in_reference_to_(rhs.in_reference_to_, alloc)
    , description_(rhs.description_, alloc)
    , amount_(rhs.amount_)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& notice = *out.mutable_pendingbailment();
    notice.set_version(Version());
    unit_.Internal().Serialize(*notice.mutable_unitid());
    notary_.Internal().Serialize(*notice.mutable_serverid());
    in_reference_to_.Internal().Serialize(*notice.mutable_requestid());
    notice.set_txid(description_.c_str());
    amount_.Serialize(writer(notice.mutable_amount()));

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::request::bailmentnotice
