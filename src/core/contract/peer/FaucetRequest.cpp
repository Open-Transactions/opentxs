// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/FaucetRequest.hpp"  // IWYU pragma: associated

#include <Faucet.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/Signable.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipient,
    opentxs::UnitType unit,
    std::string_view address,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::Faucet>
{
    using ParentType = contract::peer::implementation::Request;
    using ReturnType = contract::peer::request::implementation::Faucet;

    try {
        auto output =
            std::make_shared<ReturnType>(api, nym, recipient, unit, address);

        OT_ASSERT(output);

        auto& reply = *output;

        if (false == ParentType::Finish(reply, reason)) { return {}; }

        return output;
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::FaucetRequest(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::Faucet>
{
    using ReturnType = contract::peer::request::implementation::Faucet;

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
Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    opentxs::UnitType unit,
    std::string_view address)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          identifier::Notary{},
          PeerRequestType::Faucet)
    , unit_(unit)
    , address_(address)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , unit_(ClaimToUnit(translate(serialized.faucet().type())))
    , address_(serialized.faucet().address())
{
    Lock lock(lock_);
    init_serialized(lock);
}

Faucet::Faucet(const Faucet& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , address_(rhs.address_)
{
}

auto Faucet::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Request::IDVersion(lock);
    auto& faucet = *contract.mutable_faucet();
    faucet.set_version(version_);
    faucet.set_type(translate(UnitToClaim(unit_)));
    faucet.set_address(address_.data(), address_.size());

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
