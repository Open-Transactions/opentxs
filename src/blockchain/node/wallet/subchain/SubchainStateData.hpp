// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/BlockIndex.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Mempool.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Process.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
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
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"
#include "util/LMDB.hpp"

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
struct Mempool;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
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
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class SubchainStateData : virtual public Subchain,
                          public opentxs::Actor<SubchainStateData, SubchainJobs>
{
public:
    using WalletDatabase = node::internal::WalletDatabase;
    using Subchain = WalletDatabase::Subchain;
    using SubchainIndex = WalletDatabase::pSubchainIndex;

    const api::Session& api_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_oracle_;
    const std::function<void(const Identifier&, const char*)> task_finished_;
    const UnallocatedCString name_;
    const OTNymID owner_;
    const crypto::SubaccountType account_type_;
    const OTIdentifier id_;
    const Subchain subchain_;
    const Type chain_;
    const filter::Type filter_type_;
    const SubchainIndex db_key_;
    const block::Position null_position_;

    auto Init(boost::shared_ptr<SubchainStateData> me) noexcept -> void final
    {
        signal_startup(me);
    }
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& ancestor) noexcept -> void final;

    ~SubchainStateData() override;

protected:
    using Transactions =
        UnallocatedVector<std::shared_ptr<const block::bitcoin::Transaction>>;
    using Task = node::internal::Wallet::Task;
    using Patterns = WalletDatabase::Patterns;
    using UTXOs = UnallocatedVector<WalletDatabase::UTXO>;
    using Targets = GCS::Targets;
    using Tested = WalletDatabase::MatchingIndices;

    network::zeromq::socket::Raw& to_parent_;
    mutable BlockIndex block_index_;
    JobCounter jobs_;
    Outstanding job_counter_;
    Progress progress_;
    Process process_;
    Rescan rescan_;
    Scan scan_;

    auto describe() const noexcept -> UnallocatedCString;
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
        -> UnallocatedVector<ScriptForm>;
    auto translate(
        const UnallocatedVector<WalletDatabase::UTXO>& utxos,
        Patterns& outpoints) const noexcept -> void;
    virtual auto type() const noexcept -> std::stringstream = 0;
    auto update_scan(const block::Position& pos, bool reorg) const noexcept
        -> void;

    virtual auto get_index() noexcept -> Index& = 0;
    virtual auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) noexcept -> void = 0;
    virtual auto startup() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    virtual auto work() noexcept -> bool;

    SubchainStateData(
        const api::Session& api,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const crypto::SubaccountType accountType,
        const filter::Type filter,
        const Subchain subchain,
        const network::zeromq::BatchID batch,
        OTNymID&& owner,
        OTIdentifier&& id,
        const std::string_view shutdown,
        const std::string_view fromParent,
        const std::string_view toParent,
        allocator_type alloc) noexcept;

private:
    friend opentxs::Actor<SubchainStateData, SubchainJobs>;
    friend wallet::Index;
    friend wallet::Job;
    friend wallet::Mempool;
    friend wallet::Process;
    friend wallet::Progress;
    friend wallet::Scan;
    friend wallet::Work;

    enum class State {
        normal,
        reorg,
    };

    Mempool mempool_;
    State state_;

    auto get_targets(
        const Patterns& elements,
        const UnallocatedVector<WalletDatabase::UTXO>& utxos,
        Targets& targets) const noexcept -> void;
    auto get_targets(
        const Patterns& elements,
        const UnallocatedVector<WalletDatabase::UTXO>& utxos,
        Targets& targets,
        Patterns& outpoints,
        Tested& tested) const noexcept -> void;

    auto do_reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position ancestor) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto finish_background_tasks() noexcept -> void;
    virtual auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void = 0;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block(Message&& in) noexcept -> void;
    auto process_block(const block::Hash& block) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    auto process_filter(const block::Position& tip) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_job_finished(const Identifier& id, const char* type) noexcept
        -> void;
    auto process_mempool(Message&& in) noexcept -> void;
    auto process_mempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void;
    auto transition_state_normal(Message&& in) noexcept -> void;
    auto transition_state_reorg(Message&& in) noexcept -> void;

    SubchainStateData() = delete;
    SubchainStateData(const SubchainStateData&) = delete;
    SubchainStateData(SubchainStateData&&) = delete;
    SubchainStateData& operator=(const SubchainStateData&) = delete;
    SubchainStateData& operator=(SubchainStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
