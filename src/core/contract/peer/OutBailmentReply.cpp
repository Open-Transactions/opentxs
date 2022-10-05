// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "core/contract/peer/OutbailmentReply.hpp"  // IWYU pragma: associated

#include <OutBailmentReply.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/peer/PeerReply.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::OutBailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::Outbailment>
{
    using ParentType = contract::peer::implementation::Reply;
    using ReturnType = contract::peer::reply::implementation::Outbailment;
    auto peerRequest = proto::PeerRequest{};

    if (false == ParentType::LoadRequest(api, nym, request, peerRequest)) {
        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(
            api,
            nym,
            api.Factory().NymIDFromBase58(peerRequest.initiator()),
            request,
            server,
            terms);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::OutBailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::Outbailment>
{
    using ReturnType = contract::peer::reply::implementation::Outbailment;

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
Outbailment::Outbailment(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms)
    : Reply(
          api,
          nym,
          current_version_,
          initiator,
          server,
          PeerRequestType::OutBailment,
          request,
          terms)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Outbailment::Outbailment(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized, serialized.outbailment().instructions())
{
    Lock lock(lock_);
    init_serialized(lock);
}

Outbailment::Outbailment(const Outbailment& rhs)
    : Reply(rhs)
{
}

auto Outbailment::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Reply::IDVersion(lock);
    auto& bailment = *contract.mutable_outbailment();
    bailment.set_version(version_);
    bailment.set_instructions(conditions_);

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
