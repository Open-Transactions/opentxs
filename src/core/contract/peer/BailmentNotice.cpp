// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/BailmentNotice.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <PendingBailment.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/Signable.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto Factory::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const Amount& amount,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::request::BailmentNotice>
{
    using ParentType = contract::peer::implementation::Request;
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
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerRequest& serialized) noexcept
    -> std::shared_ptr<contract::peer::request::BailmentNotice>
{
    using ReturnType = contract::peer::request::implementation::BailmentNotice;

    if (false == proto::Validate(serialized, VERBOSE)) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Invalid serialized request.")
            .Flush();

        return {};
    }

    try {
        auto output = std::make_shared<ReturnType>(api, nym, serialized);
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
BailmentNotice::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& recipientID,
    const identifier::UnitDefinition& unitID,
    const identifier::Notary& serverID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const Amount& amount)
    : Request(
          api,
          nym,
          current_version_,
          recipientID,
          serverID,
          PeerRequestType::PendingBailment)
    , unit_(unitID)
    , server_(serverID)
    , request_id_(requestID)
    , txid_(txid)
    , amount_(amount)
{
    Lock lock(lock_);
    first_time_init(lock);
}

BailmentNotice::BailmentNotice(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Request(api, nym, serialized)
    , unit_(api_.Factory().UnitIDFromBase58(
          serialized.pendingbailment().unitid()))
    , server_(api_.Factory().NotaryIDFromBase58(
          serialized.pendingbailment().serverid()))
    , request_id_(api.Factory().IdentifierFromBase58(
          serialized.pendingbailment().requestid()))
    , txid_(serialized.pendingbailment().txid())
    , amount_(factory::Amount(serialized.pendingbailment().amount()))
{
    Lock lock(lock_);
    init_serialized(lock);
}

BailmentNotice::BailmentNotice(const BailmentNotice& rhs)
    : Request(rhs)
    , unit_(rhs.unit_)
    , server_(rhs.server_)
    , request_id_(rhs.request_id_)
    , txid_(rhs.txid_)
    , amount_(rhs.amount_)
{
}

auto BailmentNotice::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Request::IDVersion(lock);
    auto& pendingbailment = *contract.mutable_pendingbailment();
    pendingbailment.set_version(version_);
    pendingbailment.set_unitid(String::Factory(unit_)->Get());
    pendingbailment.set_serverid(String::Factory(server_)->Get());
    pendingbailment.set_requestid(String::Factory(request_id_)->Get());
    pendingbailment.set_txid(txid_);
    amount_.Serialize(writer(pendingbailment.mutable_amount()));

    return contract;
}
}  // namespace opentxs::contract::peer::request::implementation
