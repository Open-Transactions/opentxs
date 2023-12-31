// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/storesecret/Implementation.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/NoticeAcknowledgement.pb.h>
#include <opentxs/protobuf/PeerReply.pb.h>
#include <utility>

#include "opentxs/identifier/Generic.hpp"

namespace opentxs::contract::peer::reply::storesecret
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    bool value,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , value_(value)
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , value_(proto.notice().ack())
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , StoreSecretPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , value_(rhs.value_)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& storesecret = *out.mutable_notice();
    storesecret.set_version(Version());
    storesecret.set_ack(value_);

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::storesecret
