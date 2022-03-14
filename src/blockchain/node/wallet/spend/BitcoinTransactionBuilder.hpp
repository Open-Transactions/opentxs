// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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

#include "1_Internal.hpp"
#include "blockchain/node/wallet/Wallet.hpp"
#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Output;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace internal
{
struct WalletDatabase;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class BitcoinTransactionBuilder
{
public:
    using UTXO = std::pair<
        blockchain::block::Outpoint,
        std::unique_ptr<block::bitcoin::Output>>;
    using Transaction = std::unique_ptr<block::bitcoin::internal::Transaction>;
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
        const node::internal::WalletDatabase& db,
        const Identifier& id,
        const Proposal& proposal,
        const Type chain,
        const Amount feeRate) noexcept;
    ~BitcoinTransactionBuilder();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    BitcoinTransactionBuilder() = delete;
};
}  // namespace opentxs::blockchain::node::wallet
