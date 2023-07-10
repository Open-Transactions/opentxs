// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/faucet/Implementation.hpp"  // IWYU pragma: associated

#include <FaucetReply.pb.h>
#include <PeerReply.pb.h>
#include <memory>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::contract::peer::reply::faucet
{
Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    identifier::Nym initiator,
    identifier::Nym responder,
    peer::Request::identifier_type ref,
    blockchain::block::Transaction transaction,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(
          api,
          std::move(signer),
          default_version_,
          std::move(initiator),
          std::move(responder),
          std::move(ref),
          alloc)
    , txid_(transaction.ID())
    , transaction_(transaction, alloc)
    , tx_serialized_(std::nullopt)
    , accepted_(transaction_.IsValid())
    , self_(this)
{
}

Implementation::Implementation(
    const api::Session& api,
    Nym_p signer,
    const serialized_type& proto,
    allocator_type alloc) noexcept(false)
    : ReplyPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(api, std::move(signer), proto, alloc)
    , txid_(proto.faucet().txid())
    , transaction_(api_.Factory().InternalSession().BlockchainTransaction(
          proto.faucet().transaction(),
          alloc))
    , tx_serialized_(proto.faucet().transaction())
    , accepted_(transaction_.IsValid())
    , self_(this)
{
}

Implementation::Implementation(
    const Implementation& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
    , FaucetPrivate(alloc)
    , base::Implementation(rhs, alloc)
    , txid_(rhs.txid_)
    , transaction_(rhs.transaction_, alloc)
    , tx_serialized_(*rhs.tx_serialized_.lock())
    , accepted_(rhs.accepted_)
    , self_(this)
{
}

auto Implementation::id_form() const noexcept -> serialized_type
{
    auto out = base::Implementation::id_form();
    auto& faucet = *out.mutable_faucet();
    faucet.set_version(Version());
    faucet.mutable_txid()->assign(
        static_cast<const char*>(txid_.data()), txid_.size());
    auto handle = tx_serialized_.lock();
    auto& tx = *handle;

    if (false == tx.has_value()) {
        tx = transaction_.Internal().asBitcoin().Serialize(api_);
    }

    if (tx) { faucet.mutable_transaction()->CopyFrom(*tx); }

    return out;
}

Implementation::~Implementation() { Reset(self_); }
}  // namespace opentxs::contract::peer::reply::faucet
