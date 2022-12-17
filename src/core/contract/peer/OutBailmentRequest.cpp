// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/OutBailmentRequest.hpp"  // IWYU pragma: associated

#include <OutBailment.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto Factory::OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const Amount& amount,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::Outbailment>
{
    using ParentType = contract::peer::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Outbailment;

    try {
        api.Wallet().Internal().UnitDefinition(unitID);
        auto output = std::make_shared<ReturnType>(
            api, nym, recipientID, unitID, serverID, amount, terms);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::Outbailment>
{
    using ReturnType = contract::peer::request::implementation::Outbailment;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;
        Lock lock(contract.lock_);

        if (false == contract.validate(lock)) {
            LogError()("opentxs::Factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs

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
          PeerRequestType::OutBailment,
          terms)
    , unit_(unitID)
    , server_(serverID)
    , amount_(amount)
{
    Lock lock(lock_);
    first_time_init(lock);
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
    Lock lock(lock_);
    init_serialized(lock);
}

Outbailment::Outbailment(const Outbailment& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
    , amount_(rhs.amount_)
{
}

auto Outbailment::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Request::IDVersion(lock);
    auto& outbailment = *contract.mutable_outbailment();
    outbailment.set_version(version_);
    outbailment.set_unitid(String::Factory(unit_)->Get());
    outbailment.set_serverid(String::Factory(server_)->Get());
    amount_.Serialize(writer(outbailment.mutable_amount()));
    outbailment.set_instructions(conditions_);

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
