// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/connection/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ConnectionInfoReply.pb.h>
#include <opentxs/protobuf/PeerReply.pb.h>
#include <utility>

#include "opentxs/identifier/Generic.hpp"

namespace opentxs::contract::peer::reply::connection
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    bool accepted,
    std::string_view endpoint,
    std::string_view login,
    std::string_view password,
    std::string_view key,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , accepted_(accepted)
    , endpoint_(endpoint, alloc)
    , login_(login, alloc)
    , password_(password, alloc)
    , key_(key, alloc)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , accepted_(proto.connectioninfo().success())
    , endpoint_(proto.connectioninfo().url())
    , login_(proto.connectioninfo().login())
    , password_(proto.connectioninfo().password())
    , key_(proto.connectioninfo().key())
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , ConnectionPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , accepted_(rhs.accepted_)
    , endpoint_(rhs.endpoint_, alloc)
    , login_(rhs.login_, alloc)
    , password_(rhs.password_, alloc)
    , key_(rhs.key_, alloc)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& connection = *out.mutable_connectioninfo();
    connection.set_version(Version());
    connection.set_success(accepted_);
    connection.set_url(endpoint_.data(), endpoint_.size());
    connection.set_login(login_.data(), login_.size());
    connection.set_password(password_.data(), password_.size());
    connection.set_key(key_.data(), key_.size());

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::connection
