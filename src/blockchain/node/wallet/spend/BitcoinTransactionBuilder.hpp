// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainTransactionProposal.pb.h>
#include <Enums.pb.h>
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

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
namespace internal
{
class Transaction;
}  // namespace internal

class Output;
}  // namespace block
}  // namespace bitcoin

namespace database
{
class Wallet;
}  // namespace database
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class BitcoinTransactionBuilder
{
public:
    using UTXO = std::pair<
        blockchain::block::Outpoint,
        std::unique_ptr<bitcoin::block::Output>>;
    using Transaction = std::unique_ptr<bitcoin::block::internal::Transaction>;
    using KeyID = blockchain::crypto::Key;
    using Proposal = proto::BlockchainTransactionProposal;

    auto IsFunded() const noexcept -> bool;
    auto Spender() const noexcept -> const identifier::Nym&;

    auto AddChange(const Proposal& proposal) noexcept -> bool;
    auto AddInput(const UTXO& utxo) noexcept -> bool;
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
