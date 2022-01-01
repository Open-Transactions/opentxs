// Copyright (c) 2010-2021 The Open-Transactions developers
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
#include <deque>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "blockchain/node/wallet/Account.hpp"
#include "blockchain/node/wallet/Accounts.hpp"
#include "blockchain/node/wallet/DeterministicStateData.hpp"
#include "blockchain/node/wallet/Proposals.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
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
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
struct Mempool;
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
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::node::implementation
{
class Wallet final : virtual public node::internal::Wallet,
                     Worker<Wallet, api::Session>
{
public:
    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void final;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(const identifier::Nym& owner, const Identifier& subaccount)
        const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetOutputs() const noexcept -> std::pmr::vector<UTXO> final;
    auto GetOutputs(TxoState type) const noexcept
        -> std::pmr::vector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner) const noexcept
        -> std::pmr::vector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, TxoState type) const noexcept
        -> std::pmr::vector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, const Identifier& subaccount)
        const noexcept -> std::pmr::vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount,
        TxoState type) const noexcept -> std::pmr::vector<UTXO> final;
    auto GetOutputs(const crypto::Key& key, TxoState type) const noexcept
        -> std::pmr::vector<UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> std::pmr::set<TxoTag> final;
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
        const std::string& shutdown) noexcept;

    ~Wallet() final;

private:
    friend Worker<Wallet, api::Session>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        nym = value(WorkType::NymCreated),
        header = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        filter = value(WorkType::BlockchainNewFilter),
        mempool = value(WorkType::BlockchainMempoolUpdated),
        block = value(WorkType::BlockchainBlockAvailable),
        job_finished = OT_ZMQ_INTERNAL_SIGNAL + 0,
        init_wallet = OT_ZMQ_INTERNAL_SIGNAL + 1,
        key = OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    using TaskCallback = std::function<void(const Identifier&, const char*)>;

    const node::internal::Network& parent_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const TaskCallback task_finished_;
    std::atomic_bool enabled_;
    wallet::Accounts accounts_;
    wallet::Proposals proposals_;

    auto trigger_wallet() const noexcept -> void;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_block_download(const zmq::Message& in) noexcept -> void;
    auto process_block_header(const zmq::Message& in) noexcept -> void;
    auto process_filter(const zmq::Message& in) noexcept -> void;
    auto process_job_finished(const zmq::Message& in) noexcept -> void;
    auto process_key(const zmq::Message& in) noexcept -> void;
    auto process_mempool(const zmq::Message& in) noexcept -> void;
    auto process_nym(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto process_wallet() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
