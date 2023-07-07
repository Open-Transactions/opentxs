// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/request/Factory.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <exception>

#include "core/contract/peer/request/Bailment.hpp"
#include "core/contract/peer/request/BailmentNotice.hpp"
#include "core/contract/peer/request/Base.hpp"
#include "core/contract/peer/request/Connection.hpp"
#include "core/contract/peer/request/Faucet.hpp"
#include "core/contract/peer/request/Outbailment.hpp"
#include "core/contract/peer/request/StoreSecret.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const opentxs::Amount& amount,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::BailmentNotice>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::BailmentNotice;

    try {
        api.Wallet().Internal().UnitDefinition(unitID);
        auto output = std::make_shared<ReturnType>(
            api, nym, recipientID, unitID, serverID, requestID, txid, amount);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::BailmentNotice>
{
    using ReturnType = contract::peer::request::implementation::BailmentNotice;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);
        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const identifier::UnitDefinition& unit,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Bailment>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Bailment;

    try {
        api.Wallet().Internal().UnitDefinition(unit);

        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, unit, server);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Bailment>
{
    using ReturnType = contract::peer::request::implementation::Bailment;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    const contract::peer::ConnectionInfoType type,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Connection>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Connection;

    try {
        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, type, server);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto ConnectionRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Connection>
{
    using ReturnType = contract::peer::request::implementation::Connection;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Faucet>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Faucet;

    try {
        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, unit, address);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Faucet>
{
    using ReturnType = contract::peer::request::implementation::Faucet;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const opentxs::Amount& amount,
    const UnallocatedCString& terms,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Outbailment>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Outbailment;

    try {
        api.Wallet().Internal().UnitDefinition(unitID);
        auto output = std::make_shared<ReturnType>(
            api, nym, recipientID, unitID, serverID, amount, terms);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto OutbailmentRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::Outbailment>
{
    using ReturnType = contract::peer::request::implementation::Outbailment;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
                .Flush();

            return {};
        }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const contract::peer::SecretType type,
    const UnallocatedCString& primary,
    const UnallocatedCString& secondary,
    const identifier::Notary& server,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::internal::StoreSecret>
{
    using ParentType = contract::peer::request::implementation::Request;
    using ReturnType = contract::peer::request::implementation::StoreSecret;

    try {
        auto output = std::make_shared<ReturnType>(
            api, nym, recipientID, type, primary, secondary, server);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto StoreSecret(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::internal::StoreSecret>
{
    using ReturnType = contract::peer::request::implementation::StoreSecret;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);

        OT_ASSERT(output);

        auto& contract = *output;

        if (false == contract.Validate()) {
            LogError()("opentxs::factory::")(__func__)(": Invalid request.")
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
