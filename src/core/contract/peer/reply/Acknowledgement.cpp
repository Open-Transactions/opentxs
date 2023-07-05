// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/reply/Acknowledgement.hpp"  // IWYU pragma: associated

#include <NoticeAcknowledgement.pb.h>
#include <PeerReply.pb.h>

#include "core/contract/peer/reply/Base.hpp"

namespace opentxs::contract::peer::reply::implementation
{
Acknowledgement::Acknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const contract::peer::RequestType type,
    const bool& ack)
    : Reply(api, nym, current_version_, initiator, server, type, request)
    , ack_(ack)
{
    first_time_init(set_name_from_id_);
}

Acknowledgement::Acknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized)
    , ack_(serialized.notice().ack())
{
    init_serialized();
}

Acknowledgement::Acknowledgement(const Acknowledgement& rhs)
    : Reply(rhs)
    , ack_(rhs.ack_)
{
}

auto Acknowledgement::IDVersion() const -> SerializedType
{
    auto contract = Reply::IDVersion();
    auto& notice = *contract.mutable_notice();
    notice.set_version(Version());
    notice.set_ack(ack_);

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
