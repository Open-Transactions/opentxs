// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/request/Connection.hpp"  // IWYU pragma: associated

#include <ConnectionInfo.pb.h>
#include <PeerRequest.pb.h>

#include "core/contract/peer/request/Base.hpp"
#include "internal/core/contract/peer/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"              // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

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
          contract::peer::RequestType::ConnectionInfo)
    , connection_type_(type)
{
    first_time_init(set_name_from_id_);
}

Connection::Connection(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , connection_type_(translate(serialized.connectioninfo().type()))
{
    init_serialized();
}

Connection::Connection(const Connection& rhs)
    : Request(rhs)
    , connection_type_(rhs.connection_type_)
{
}

auto Connection::IDVersion() const -> SerializedType
{
    auto contract = Request::IDVersion();
    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(Version());
    connectioninfo.set_type(translate(connection_type_));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
