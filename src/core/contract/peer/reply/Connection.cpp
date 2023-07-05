// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/reply/Connection.hpp"  // IWYU pragma: associated

#include <ConnectionInfoReply.pb.h>
#include <PeerReply.pb.h>

#include "core/contract/peer/reply/Base.hpp"
#include "opentxs/api/session/Crypto.hpp"              // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::reply::implementation
{
Connection::Connection(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const bool ack,
    const UnallocatedCString& url,
    const UnallocatedCString& login,
    const UnallocatedCString& password,
    const UnallocatedCString& key)
    : Reply(
          api,
          nym,
          current_version_,
          initiator,
          server,
          contract::peer::RequestType::ConnectionInfo,
          request)
    , success_(ack)
    , url_(url)
    , login_(login)
    , password_(password)
    , key_(key)
{
    first_time_init(set_name_from_id_);
}

Connection::Connection(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized)
    , success_(serialized.connectioninfo().success())
    , url_(serialized.connectioninfo().url())
    , login_(serialized.connectioninfo().login())
    , password_(serialized.connectioninfo().password())
    , key_(serialized.connectioninfo().key())
{
    init_serialized();
}

Connection::Connection(const Connection& rhs)
    : Reply(rhs)
    , success_(rhs.success_)
    , url_(rhs.url_)
    , login_(rhs.login_)
    , password_(rhs.password_)
    , key_(rhs.key_)
{
}

auto Connection::IDVersion() const -> SerializedType
{
    auto contract = Reply::IDVersion();
    auto& connectioninfo = *contract.mutable_connectioninfo();
    connectioninfo.set_version(Version());
    connectioninfo.set_success(success_);
    connectioninfo.set_url(url_);
    connectioninfo.set_login(login_);
    connectioninfo.set_password(password_);
    connectioninfo.set_key(key_);

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
