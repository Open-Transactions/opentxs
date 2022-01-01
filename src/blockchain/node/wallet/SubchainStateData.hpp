// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/Actor.hpp"
#include "blockchain/node/wallet/BlockIndex.hpp"
#include "blockchain/node/wallet/Mempool.hpp"
#include "blockchain/node/wallet/Process.hpp"
#include "blockchain/node/wallet/Progress.hpp"
#include "blockchain/node/wallet/Rescan.hpp"
#include "blockchain/node/wallet/Scan.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
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
class Block;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace crypto
{
class Element;
}  // namespace crypto

namespace node
{
namespace internal
{
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Accounts;
class Index;
class Job;
class ScriptForm;
class Work;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Outstanding;
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class SubchainStateData : virtual public Actor
{
public:
    using WalletDatabase = node::internal::WalletDatabase;
    using Subchain = WalletDatabase::Subchain;
    using SubchainIndex = WalletDatabase::pSubchainIndex;

    const api::Session& api_;
    const node::internal::Network& node_;
    Accounts& parent_;
    const WalletDatabase& db_;
    const std::function<void(const Identifier&, const char*)>& task_finished_;
    const std::string name_;
    const OTNymID owner_;
    const crypto::SubaccountType account_type_;
    const OTIdentifier id_;
    const Subchain subchain_;
    const filter::Type filter_type_;
    const SubchainIndex db_key_;
    const block::Position null_position_;

    auto FinishBackgroundTasks() noexcept -> void final;
    auto ProcessBlockAvailable(const block::Hash& block) noexcept -> void final;
    auto ProcessKey() noexcept -> void final;
    auto ProcessMempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void final;
    auto ProcessNewFilter(const block::Position& tip) noexcept -> void final;
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& ancestor) noexcept -> bool final;
    auto ProcessStateMachine(bool enabled) noexcept -> bool final;
    auto ProcessTaskComplete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    ~SubchainStateData() override;

protected:
    using Transactions =
        std::pmr::vector<std::shared_ptr<const block::bitcoin::Transaction>>;
    using Task = node::internal::Wallet::Task;
    using Patterns = WalletDatabase::Patterns;
    using UTXOs = std::pmr::vector<WalletDatabase::UTXO>;
    using Targets = GCS::Targets;
    using Tested = WalletDatabase::MatchingIndices;

    Outstanding& job_counter_;
    mutable BlockIndex block_index_;
    Progress progress_;
    Process process_;
    Rescan rescan_;
    Scan scan_;

    auto describe() const noexcept -> std::string;
    auto get_account_targets() const noexcept
        -> std::tuple<Patterns, UTXOs, Targets>;
    auto get_block_targets(const block::Hash& id, const UTXOs& utxos)
        const noexcept -> std::pair<Patterns, Targets>;
    auto get_block_targets(const block::Hash& id, Tested& tested) const noexcept
        -> std::tuple<Patterns, UTXOs, Targets, Patterns>;
    auto index_element(
        const filter::Type type,
        const blockchain::crypto::Element& input,
        const Bip32Index index,
        WalletDatabase::ElementMap& output) const noexcept -> void;
    virtual auto report_scan(const block::Position& pos) const noexcept -> void;
    auto set_key_data(block::bitcoin::Transaction& tx) const noexcept -> void;
    auto supported_scripts(const crypto::Element& element) const noexcept
        -> std::pmr::vector<ScriptForm>;
    auto translate(
        const std::pmr::vector<WalletDatabase::UTXO>& utxos,
        Patterns& outpoints) const noexcept -> void;
    virtual auto type() const noexcept -> std::stringstream = 0;
    auto update_scan(const block::Position& pos, bool reorg) const noexcept
        -> void;

    virtual auto get_index() noexcept -> Index& = 0;
    virtual auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) noexcept -> void = 0;
    // NOTE call from all and only final constructor bodies
    auto init() noexcept -> void;

    SubchainStateData(
        const api::Session& api,
        const node::internal::Network& node,
        Accounts& parent,
        const WalletDatabase& db,
        OTNymID&& owner,
        crypto::SubaccountType accountType,
        OTIdentifier&& id,
        const std::function<void(const Identifier&, const char*)>& taskFinished,
        Outstanding& jobCounter,
        const filter::Type filter,
        const Subchain subchain) noexcept;

private:
    friend Index;
    friend Job;
    friend Mempool;
    friend Process;
    friend Progress;
    friend Scan;
    friend Work;

    Mempool mempool_;

    auto get_targets(
        const Patterns& elements,
        const std::pmr::vector<WalletDatabase::UTXO>& utxos,
        Targets& targets) const noexcept -> void;
    auto get_targets(
        const Patterns& elements,
        const std::pmr::vector<WalletDatabase::UTXO>& utxos,
        Targets& targets,
        Patterns& outpoints,
        Tested& tested) const noexcept -> void;

    auto do_reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position ancestor) noexcept -> void;
    virtual auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void = 0;

    SubchainStateData() = delete;
    SubchainStateData(const SubchainStateData&) = delete;
    SubchainStateData(SubchainStateData&&) = delete;
    SubchainStateData& operator=(const SubchainStateData&) = delete;
    SubchainStateData& operator=(SubchainStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
