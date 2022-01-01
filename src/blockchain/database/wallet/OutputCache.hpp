// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <robin_hood.h>
#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "blockchain/database/wallet/Output.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
namespace internal
{
struct Output;
}  // namespace internal

class Output;
}  // namespace bitcoin

struct Outpoint;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
constexpr auto accounts_{Table::AccountOutputs};
constexpr auto config_{Table::Config};
constexpr auto generation_{Table::GenerationOutputs};
constexpr auto keys_{Table::KeyOutputs};
constexpr auto nyms_{Table::NymOutputs};
constexpr auto output_proposal_{Table::OutputProposals};
constexpr auto outputs_{Table::WalletOutputs};
constexpr auto positions_{Table::PositionOutputs};
constexpr auto proposal_created_{Table::ProposalCreatedOutputs};
constexpr auto proposal_spent_{Table::ProposalSpentOutputs};
constexpr auto states_{Table::StateOutputs};
constexpr auto subchains_{Table::SubchainOutputs};

using Dir = storage::lmdb::LMDB::Dir;
using Mode = storage::lmdb::LMDB::Mode;
using SubchainID = Identifier;
using pSubchainID = OTIdentifier;
using States = std::pmr::vector<node::TxoState>;
using Matches = std::pmr::vector<block::Outpoint>;
using Outpoints = robin_hood::unordered_node_set<block::Outpoint>;
using NymBalances = std::pmr::map<OTNymID, Balance>;
using Nyms = robin_hood::unordered_node_set<OTNymID>;

auto all_states() noexcept -> const States&;

template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

class OutputCache
{
public:
    auto Print(const eLock&) const noexcept -> void;

    auto AddOutput(
        const eLock&,
        const block::Outpoint& id,
        MDB_txn* tx,
        std::unique_ptr<block::bitcoin::Output> output) noexcept -> bool;
    auto AddToAccount(
        const eLock&,
        const AccountID& id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto AddToKey(
        const eLock&,
        const crypto::Key& id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto AddToNym(
        const eLock&,
        const identifier::Nym& id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto AddToPosition(
        const eLock&,
        const block::Position& id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto AddToState(
        const eLock&,
        const node::TxoState id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto AddToSubchain(
        const eLock&,
        const SubchainID& id,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto ChangePosition(
        const eLock&,
        const block::Position& oldPosition,
        const block::Position& newPosition,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto ChangeState(
        const eLock&,
        const node::TxoState oldState,
        const node::TxoState newState,
        const block::Outpoint& output,
        MDB_txn* tx) noexcept -> bool;
    auto Clear(const eLock&) noexcept -> void;
    auto GetAccount(const sLock&, const AccountID& id) noexcept
        -> const Outpoints&;
    auto GetAccount(const eLock&, const AccountID& id) noexcept
        -> const Outpoints&;
    auto GetKey(const sLock&, const crypto::Key& id) noexcept
        -> const Outpoints&;
    auto GetKey(const eLock&, const crypto::Key& id) noexcept
        -> const Outpoints&;
    auto GetHeight() noexcept -> block::Height;
    auto GetNym(const sLock&, const identifier::Nym& id) noexcept
        -> const Outpoints&;
    auto GetNym(const eLock&, const identifier::Nym& id) noexcept
        -> const Outpoints&;
    auto GetNyms() noexcept -> const Nyms&;
    auto GetNyms(const eLock&) noexcept -> const Nyms&;
    auto GetOutput(const sLock&, const block::Outpoint& id) noexcept(false)
        -> const block::bitcoin::internal::Output&;
    auto GetOutput(const eLock&, const block::Outpoint& id) noexcept(false)
        -> block::bitcoin::internal::Output&;
    auto GetPosition(const eLock&) noexcept -> const db::Position&;
    auto GetPosition(const sLock&, const block::Position& id) noexcept
        -> const Outpoints&;
    auto GetPosition(const eLock&, const block::Position& id) noexcept
        -> const Outpoints&;
    auto GetState(const sLock&, const node::TxoState id) noexcept
        -> const Outpoints&;
    auto GetState(const eLock&, const node::TxoState id) noexcept
        -> const Outpoints&;
    auto GetSubchain(const sLock&, const SubchainID& id) noexcept
        -> const Outpoints&;
    auto GetSubchain(const eLock&, const SubchainID& id) noexcept
        -> const Outpoints&;
    auto UpdateOutput(
        const eLock&,
        const block::Outpoint& id,
        const block::bitcoin::Output& output,
        MDB_txn* tx) noexcept -> bool;
    auto UpdatePosition(
        const eLock&,
        const block::Position&,
        MDB_txn* tx) noexcept -> bool;

    OutputCache(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const block::Position& blank) noexcept;

    ~OutputCache();

private:
    static constexpr std::size_t reserve_{10000u};
    static const Outpoints empty_outputs_;
    static const Nyms empty_nyms_;

    // NOTE if an exclusive lock is being held in the parent then locking
    // these mutexes is redundant
    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    const blockchain::Type chain_;
    const block::Position& blank_;
    std::mutex position_lock_;
    std::optional<db::Position> position_;
    std::mutex output_lock_;
    robin_hood::unordered_node_map<
        block::Outpoint,
        std::unique_ptr<block::bitcoin::Output>>
        outputs_;
    std::mutex account_lock_;
    robin_hood::unordered_node_map<OTIdentifier, Outpoints> accounts_;
    std::mutex key_lock_;
    robin_hood::unordered_node_map<crypto::Key, Outpoints> keys_;
    std::mutex nym_lock_;
    robin_hood::unordered_node_map<OTNymID, Outpoints> nyms_;
    std::optional<Nyms> nym_list_;
    std::mutex positions_lock_;
    robin_hood::unordered_node_map<block::Position, Outpoints> positions_;
    std::mutex state_lock_;
    robin_hood::unordered_node_map<node::TxoState, Outpoints> states_;
    std::mutex subchain_lock_;
    robin_hood::unordered_node_map<OTIdentifier, Outpoints> subchains_;

    auto get_position() noexcept -> const db::Position&;
    auto load_output(const block::Outpoint& id) noexcept(false)
        -> block::bitcoin::internal::Output&;
    template <typename MapKeyType, typename DBKeyType, typename MapType>
    auto load_output_index(
        const Table table,
        const MapKeyType& key,
        const DBKeyType dbKey,
        const char* indexName,
        const std::string& keyName,
        MapType& map) noexcept -> Outpoints&;
    auto load_nyms() noexcept -> Nyms&;
    auto load_position() noexcept -> void;
    auto write_output(
        const block::Outpoint& id,
        const block::bitcoin::Output& output,
        MDB_txn* tx) noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::wallet
