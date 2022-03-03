// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

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
#include <string_view>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "blockchain/node/wallet/spend/Proposals.hpp"
#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
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
namespace node
{
class Mempool;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class Wallet final : virtual public node::internal::Wallet,
                     Worker<Wallet, api::Session>
{
public:
    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void final;
    auto FeeEstimate() const noexcept -> std::optional<Amount> final
    {
        return fee_oracle_.EstimatedFee();
    }
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(const identifier::Nym& owner, const Identifier& subaccount)
        const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetOutputs() const noexcept -> UnallocatedVector<UTXO> final;
    auto GetOutputs(TxoState type) const noexcept
        -> UnallocatedVector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner) const noexcept
        -> UnallocatedVector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, TxoState type) const noexcept
        -> UnallocatedVector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, const Identifier& subaccount)
        const noexcept -> UnallocatedVector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount,
        TxoState type) const noexcept -> UnallocatedVector<UTXO> final;
    auto GetOutputs(const crypto::Key& key, TxoState type) const noexcept
        -> UnallocatedVector<UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final;
    auto Height() const noexcept -> block::Height final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return signal_shutdown();
    }

    Wallet(
        const api::Session& api,
        const node::internal::Network& parent,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const Type chain,
        const std::string_view shutdown) noexcept;

    ~Wallet() final;

private:
    friend Worker<Wallet, api::Session>;

    using Work = wallet::WalletJobs;

    const node::internal::Network& parent_;
    const node::internal::WalletDatabase& db_;
    const Type chain_;
    network::zeromq::socket::Raw& to_accounts_;
    wallet::FeeOracle fee_oracle_;
    wallet::Accounts accounts_;
    wallet::Proposals proposals_;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    Wallet(
        const api::Session& api,
        const node::internal::Network& parent,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const Type chain,
        const std::string_view shutdown,
        const CString accounts) noexcept;
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
