// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransactionProposal.pb.h>
#include <memory>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace database
{
class Wallet;
}  // namespace database
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class BitcoinTransactionBuilder
{
public:
    using Transaction = bitcoin::block::Transaction;
    using KeyID = blockchain::crypto::Key;
    using Proposal = proto::BlockchainTransactionProposal;

    auto IsFunded() const noexcept -> bool;
    auto Spender() const noexcept -> const identifier::Nym&;

    auto AddChange(const Proposal& proposal) noexcept -> bool;
    auto AddInput(const UTXO& utxo) noexcept -> bool;
    auto CreateNotifications(const Proposal& proposal) noexcept -> bool;
    auto CreateOutputs(const Proposal& proposal) noexcept -> bool;
    auto FinalizeOutputs() noexcept -> void;
    auto FinalizeTransaction() noexcept -> Transaction;
    auto ReleaseKeys() noexcept -> void;
    auto SignInputs() noexcept -> bool;

    BitcoinTransactionBuilder(
        const api::Session& api,
        database::Wallet& db,
        const identifier::Generic& id,
        const Proposal& proposal,
        const Type chain,
        const Amount feeRate) noexcept;
    BitcoinTransactionBuilder() = delete;

    ~BitcoinTransactionBuilder();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
