// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/NoticeAcknowledgement.hpp"  // IWYU pragma: associated

#include <NoticeAcknowledgement.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/Signable.hpp"
#include "core/contract/peer/PeerReply.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const contract::peer::PeerRequestType type,
    const bool& ack,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::Acknowledgement>
{
    using ParentType = contract::peer::implementation::Reply;
    using ReturnType = contract::peer::reply::implementation::Acknowledgement;

    try {
        auto peerRequest = proto::PeerRequest{};
        if (false == ParentType::LoadRequest(api, nym, request, peerRequest)) {
            return {};
        }

        auto output = std::make_shared<ReturnType>(
            api,
            nym,
            api.Factory().NymIDFromBase58(peerRequest.initiator()),
            request,
            server,
            type,
            ack);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::Acknowledgement>
{
    using ReturnType = contract::peer::reply::implementation::Acknowledgement;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;
        Lock lock(contract.lock_);

        if (false == contract.validate(lock)) {
            LogError()("opentxs::Factory::")(__func__)(": Invalid reply.")
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

namespace opentxs::contract::peer::reply::implementation
{
Acknowledgement::Acknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const contract::peer::PeerRequestType type,
    const bool& ack)
    : Reply(api, nym, current_version_, initiator, server, type, request)
    , ack_(ack)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Acknowledgement::Acknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized)
    , ack_(serialized.notice().ack())
{
    Lock lock(lock_);
    init_serialized(lock);
}

Acknowledgement::Acknowledgement(const Acknowledgement& rhs)
    : Reply(rhs)
    , ack_(rhs.ack_)
{
}

auto Acknowledgement::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Reply::IDVersion(lock);
    auto& notice = *contract.mutable_notice();
    notice.set_version(version_);
    notice.set_ack(ack_);

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
