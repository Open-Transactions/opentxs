// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::PasswordPrompt

#include "core/contract/peer/reply/Faucet.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <FaucetReply.pb.h>
#include <PeerReply.pb.h>
#include <memory>
#include <optional>

#include "core/contract/peer/reply/Base.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"

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
          RequestType::Faucet,
          request)
    , txid_(transaction.ID())
    , transaction_(transaction)
    , tx_serialized_(std::nullopt)
{
    first_time_init(set_name_from_id_);
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
    init_serialized();
}

Faucet::Faucet(const Faucet& rhs)
    : Reply(rhs)
    , txid_(rhs.txid_)
    , transaction_(rhs.transaction_)
    , tx_serialized_(*rhs.tx_serialized_.lock())
{
}

auto Faucet::IDVersion() const -> SerializedType
{
    auto contract = Reply::IDVersion();
    auto& faucet = *contract.mutable_faucet();
    faucet.set_version(Version());
    faucet.mutable_txid()->assign(
        static_cast<const char*>(txid_.data()), txid_.size());
    auto handle = tx_serialized_.lock();
    auto& tx = *handle;

    if (false == tx.has_value()) {
        tx = transaction_.Internal().asBitcoin().Serialize(api_);
    }

    if (tx) { faucet.mutable_transaction()->CopyFrom(*tx); }

    return contract;
}
}  // namespace opentxs::contract::peer::reply::implementation
