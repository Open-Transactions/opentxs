// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/BailmentNotice.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <PendingBailment.pb.h>

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
BailmentNotice::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const Amount& amount)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          RequestType::PendingBailment)
    , unit_(unitID)
    , server_(serverID)
    , request_id_(requestID)
    , txid_(txid)
    , amount_(amount)
{
    first_time_init(set_name_from_id_);
}

BailmentNotice::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , unit_(api_.Factory().UnitIDFromBase58(
          serialized.pendingbailment().unitid()))
    , server_(api_.Factory().NotaryIDFromBase58(
          serialized.pendingbailment().serverid()))
    , request_id_(api.Factory().IdentifierFromBase58(
          serialized.pendingbailment().requestid()))
    , txid_(serialized.pendingbailment().txid())
    , amount_(factory::Amount(serialized.pendingbailment().amount()))
{
    init_serialized();
}

BailmentNotice::BailmentNotice(const BailmentNotice& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
    , request_id_(rhs.request_id_)
    , txid_(rhs.txid_)
    , amount_(rhs.amount_)
{
}

auto BailmentNotice::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& pendingbailment = *contract.mutable_pendingbailment();
    pendingbailment.set_version(Version());
    pendingbailment.set_unitid(String::Factory(unit_, api_.Crypto())->Get());
    pendingbailment.set_serverid(
        String::Factory(server_, api_.Crypto())->Get());
    pendingbailment.set_requestid(
        String::Factory(request_id_, api_.Crypto())->Get());
    pendingbailment.set_txid(txid_);
    amount_.Serialize(writer(pendingbailment.mutable_amount()));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
