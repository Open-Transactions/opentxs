// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "core/contract/peer/BailmentRequest.hpp"  // IWYU pragma: associated

#include <Bailment.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "2_Factory.hpp"
#include "Proto.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::Bailment>
{
    using ParentType = contract::peer::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Bailment;

    try {
        api.Wallet().UnitDefinition(unit);

        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, unit, server);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::Bailment>
{
    using ReturnType = contract::peer::request::implementation::Bailment;

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
          PeerRequestType::Bailment)
    , unit_(unitID)
    , server_(serverID)
{
    Lock lock(lock_);
    first_time_init(lock);
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
    Lock lock(lock_);
    init_serialized(lock);
}

Bailment::Bailment(const Bailment& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
{
}

auto Bailment::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Request::IDVersion(lock);
    auto& bailment = *contract.mutable_bailment();
    bailment.set_version(version_);
    bailment.set_unitid(unit_.asBase58(api_.Crypto()));
    bailment.set_serverid(server_.asBase58(api_.Crypto()));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
