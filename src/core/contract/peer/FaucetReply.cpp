// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/FaucetReply.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <FaucetReply.pb.h>
#include <PeerReply.pb.h>
#include <PeerRequest.pb.h>
#include <memory>
#include <optional>
#include <stdexcept>

#include "2_Factory.hpp"
#include "core/contract/Signable.hpp"
#include "core/contract/peer/PeerReply.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PeerReply.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/contract/peer/PeerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const blockchain::block::Transaction& transaction,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::shared_ptr<contract::peer::reply::Faucet>
{
    using ParentType = contract::peer::implementation::Reply;
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
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto Factory::FaucetReply(
    const api::Session& api,
    const Nym_p& nym,
    const proto::PeerReply& serialized) noexcept
    -> std::shared_ptr<contract::peer::reply::Faucet>
{
    using ReturnType = contract::peer::reply::implementation::Faucet;

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
Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const identifier::Nym& initiator,
    const identifier::Generic& request,
    const blockchain::block::Transaction& transaction)
    : Reply(
          api,
          nym,
          current_version_,
          initiator,
          identifier::Notary{},
          PeerRequestType::Faucet,
          request)
    , txid_(transaction.ID())
    , transaction_(transaction)
    , tx_serialized_(std::nullopt)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Faucet::Faucet(
    const api::Session& api,
    const Nym_p& nym,
    const SerializedType& serialized)
    : Reply(api, nym, serialized)
    , txid_(serialized.faucet().txid())
    , transaction_(api_.Factory().InternalSession().BlockchainTransaction(
          serialized.faucet().transaction(),
          {}))
    , tx_serialized_(serialized.faucet().transaction())
{
    Lock lock(lock_);
    init_serialized(lock);
}

Faucet::Faucet(const Faucet& rhs)
    : Reply(rhs)
    , txid_(rhs.txid_)
    , transaction_(rhs.transaction_)
    , tx_serialized_(rhs.tx_serialized_)
{
}

auto Faucet::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Reply::IDVersion(lock);
    auto& faucet = *contract.mutable_faucet();
    faucet.set_version(version_);
    faucet.mutable_txid()->assign(
        static_cast<const char*>(txid_.data()), txid_.size());

    if (false == tx_serialized_.has_value()) {
        tx_serialized_ = transaction_.Internal().asBitcoin().Serialize(api_);
    }

    if (tx_serialized_) { *faucet.mutable_transaction() = *tx_serialized_; }

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
