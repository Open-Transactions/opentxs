// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/reply/Factory.hpp"  // IWYU pragma: associated

#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <exception>

#include "core/contract/peer/reply/Acknowledgement.hpp"
#include "core/contract/peer/reply/Bailment.hpp"
#include "core/contract/peer/reply/Base.hpp"
#include "core/contract/peer/reply/Connection.hpp"
#include "core/contract/peer/reply/Faucet.hpp"
#include "core/contract/peer/reply/Outbailment.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Bailment>
{
    using ParentType = contract::peer::reply::implementation::Reply;
    using ReturnType = contract::peer::reply::implementation::Bailment;
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
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Bailment>
{
    using ReturnType = contract::peer::reply::implementation::Bailment;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid reply.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto ConnectionReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const bool ack,
    const UnallocatedCString& url,
    const UnallocatedCString& login,
    const UnallocatedCString& password,
    const UnallocatedCString& key,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Connection>
{
    using ParentType = contract::peer::reply::implementation::Reply;
    using ReturnType = contract::peer::reply::implementation::Connection;
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
            ack,
            url,
            login,
            password,
            key);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto ConnectionReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Connection>
{
    using ReturnType = contract::peer::reply::implementation::Connection;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid reply.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Faucet>
{
    using ParentType = contract::peer::reply::implementation::Reply;
    using ReturnType = contract::peer::reply::implementation::Faucet;
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
            transaction);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Faucet>
{
    using ReturnType = contract::peer::reply::implementation::Faucet;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid reply.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const contract::peer::RequestType type,
    const bool& ack,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Acknowledgement>
{
    using ParentType = contract::peer::reply::implementation::Reply;
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
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto NoticeAcknowledgement(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Acknowledgement>
{
    using ReturnType = contract::peer::reply::implementation::Acknowledgement;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid reply.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto OutBailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const identifier::Notary& server,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Outbailment>
{
    using ParentType = contract::peer::reply::implementation::Reply;
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
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto OutBailmentReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::internal::Outbailment>
{
    using ReturnType = contract::peer::reply::implementation::Outbailment;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized reply.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid reply.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
