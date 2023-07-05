// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/Outbailment.hpp"  // IWYU pragma: associated

#include <OutBailment.pb.h>
#include <PeerRequest.pb.h>

#include "core/contract/peer/request/Base.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::contract::peer::request::implementation
{
Outbailment::Outbailment(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const Amount& amount,
    const UnallocatedCString& terms)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          RequestType::OutBailment,
          terms)
    , unit_(unitID)
    , server_(serverID)
    , amount_(amount)
{
    first_time_init(set_name_from_id_);
}

Outbailment::Outbailment(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized, serialized.outbailment().instructions())
    , unit_(api_.Factory().UnitIDFromBase58(serialized.outbailment().unitid()))
    , server_(api_.Factory().NotaryIDFromBase58(
          serialized.outbailment().serverid()))
    , amount_(factory::Amount(serialized.outbailment().amount()))
{
    init_serialized();
}

Outbailment::Outbailment(const Outbailment& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
    , amount_(rhs.amount_)
{
}

auto Outbailment::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& outbailment = *contract.mutable_outbailment();
    outbailment.set_version(Version());
    outbailment.set_unitid(String::Factory(unit_, api_.Crypto())->Get());
    outbailment.set_serverid(String::Factory(server_, api_.Crypto())->Get());
    amount_.Serialize(writer(outbailment.mutable_amount()));
    outbailment.set_instructions(UnallocatedCString{Terms()});

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
