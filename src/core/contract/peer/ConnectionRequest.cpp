// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "core/contract/peer/ConnectionRequest.hpp"  // IWYU pragma: associated

#include <ConnectionInfo.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/core/contract/peer/Peer.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::ConnectionInfoType type,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::Connection>
{
    using ParentType = contract::peer::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Connection;

    try {
        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, type, server);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::Connection>
{
    using ReturnType = contract::peer::request::implementation::Connection;

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
Connection::Connection(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const contract::peer::ConnectionInfoType type,
    const identifier::Notary& serverID)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          contract::peer::PeerRequestType::ConnectionInfo)
    , connection_type_(type)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Connection::Connection(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , connection_type_(translate(serialized.connectioninfo().type()))
{
    Lock lock(lock_);
    init_serialized(lock);
}

Connection::Connection(const Connection& rhs)
    : Request(rhs)
    , connection_type_(rhs.connection_type_)
{
}

auto Connection::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Request::IDVersion(lock);
    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(version_);
    connectioninfo.set_type(translate(connection_type_));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
