// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/Bailment.hpp"  // IWYU pragma: associated

#include <Bailment.pb.h>
#include <PeerRequest.pb.h>

#include "core/contract/peer/request/Base.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs::contract::peer::request::implementation
{
Bailment::Bailment(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          RequestType::Bailment)
    , unit_(unitID)
    , server_(serverID)
{
    first_time_init(set_name_from_id_);
}

Bailment::Bailment(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , unit_(api_.Factory().UnitIDFromBase58(serialized.bailment().unitid()))
    , server_(
          api_.Factory().NotaryIDFromBase58(serialized.bailment().serverid()))
{
    init_serialized();
}

Bailment::Bailment(const Bailment& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
{
}

auto Bailment::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(Version());
    bailment.set_unitid(unit_.asBase58(api_.Crypto()));
    bailment.set_serverid(server_.asBase58(api_.Crypto()));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
