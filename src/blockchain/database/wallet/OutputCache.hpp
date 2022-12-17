// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoState
// IWYU pragma: no_forward_declare opentxs::storage::lmdb::Dir
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <ankerl/unordered_dense.h>
#include <robin_hood.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>

#include "blockchain/database/wallet/Output.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

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
class Output;
}  // namespace internal

class Output;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
class Position;
}  // namespace block
}  // namespace blockchain

namespace storage
{
namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
constexpr auto accounts_{Table::AccountOutputs};
constexpr auto generation_{Table::GenerationOutputs};
constexpr auto keys_{Table::KeyOutputs};
constexpr auto nyms_{Table::NymOutputs};
constexpr auto output_config_{Table::Config};
constexpr auto output_proposal_{Table::OutputProposals};
constexpr auto outputs_{Table::WalletOutputs};
constexpr auto positions_{Table::PositionOutputs};
constexpr auto proposal_created_{Table::ProposalCreatedOutputs};
constexpr auto proposal_spent_{Table::ProposalSpentOutputs};
constexpr auto states_{Table::StateOutputs};
constexpr auto subchains_{Table::SubchainOutputs};

using Dir = storage::lmdb::Dir;
using Mode = storage::lmdb::Mode;
using States = UnallocatedVector<node::TxoState>;
using Matches = UnallocatedVector<block::Outpoint>;
using Outpoints = ankerl::unordered_dense::set<block::Outpoint>;
using NymBalances = UnallocatedMap<identifier::Nym, Balance>;
using Nyms = ankerl::unordered_dense::set<identifier::Nym>;

auto all_states() noexcept -> const States&;

class OutputCache
{
public:
    auto Exists(const block::Outpoint& id) const noexcept -> bool;
    auto Exists(const SubchainID& subchain, const block::Outpoint& id)
        const noexcept -> bool;
    auto GetAccount(const AccountID& id) const noexcept -> const Outpoints&;
    auto GetKey(const crypto::Key& id) const noexcept -> const Outpoints&;
    auto GetHeight() const noexcept -> block::Height;
    auto GetNym(const identifier::Nym& id) const noexcept -> const Outpoints&;
    auto GetNyms() const noexcept -> const Nyms&;
    auto GetOutput(const block::Outpoint& id) const noexcept(false)
        -> const bitcoin::block::Output&;
    auto GetPosition() const noexcept -> const db::Position&;
    auto GetPosition(const block::Position& id) const noexcept
        -> const Outpoints&;
    auto GetState(const node::TxoState id) const noexcept -> const Outpoints&;
    auto GetSubchain(const SubchainID& id) const noexcept -> const Outpoints&;
    auto Populate() const noexcept -> void;
    auto Print() const noexcept -> void;

    auto AddOutput(
        const block::Outpoint& id,
        storage::lmdb::Transaction& tx,
        bitcoin::block::Output output) noexcept -> bool;
    auto AddOutput(
        const block::Outpoint& id,
        const node::TxoState state,
        const block::Position& position,
        const AccountID& account,
        const SubchainID& subchain,
        storage::lmdb::Transaction& tx,
        bitcoin::block::Output output) noexcept -> bool;
    auto AddToAccount(
        const AccountID& id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddToKey(
        const crypto::Key& id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddToNym(
        const identifier::Nym& id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddToPosition(
        const block::Position& id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddToState(
        const node::TxoState id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto AddToSubchain(
        const SubchainID& id,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto ChangePosition(
        const block::Position& oldPosition,
        const block::Position& newPosition,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto ChangeState(
        const node::TxoState oldState,
        const node::TxoState newState,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto Clear() noexcept -> void;
    auto GetOutput(const block::Outpoint& id) noexcept(false)
        -> bitcoin::block::Output&;
    auto GetOutput(
        const SubchainID& subchain,
        const block::Outpoint& id) noexcept(false) -> bitcoin::block::Output&;
    auto UpdateOutput(
        const block::Outpoint& id,
        const bitcoin::block::Output& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
    auto UpdatePosition(
        const block::Position&,
        storage::lmdb::Transaction& tx) noexcept -> bool;

    OutputCache(
        const api::Session& api,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type chain,
        const block::Position& blank) noexcept;

    ~OutputCache();

private:
    static constexpr std::size_t reserve_{10000u};
    static const Outpoints empty_outputs_;
    static const Nyms empty_nyms_;

    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    const blockchain::Type chain_;
    const block::Position& blank_;
    std::optional<db::Position> position_;
    ankerl::unordered_dense::map<block::Outpoint, bitcoin::block::Output>
        outputs_;
    ankerl::unordered_dense::map<identifier::Generic, Outpoints> accounts_;
    ankerl::unordered_dense::map<crypto::Key, Outpoints> keys_;
    ankerl::unordered_dense::map<identifier::Nym, Outpoints> nyms_;
    Nyms nym_list_;
    ankerl::unordered_dense::map<block::Position, Outpoints> positions_;
    ankerl::unordered_dense::map<node::TxoState, Outpoints> states_;
    ankerl::unordered_dense::map<identifier::Generic, Outpoints> subchains_;
    bool populated_;

    auto get_position() const noexcept -> const db::Position&;
    auto load_output(const block::Outpoint& id) const noexcept(false)
        -> const bitcoin::block::Output&;
    template <typename MapKeyType, typename MapType>
    auto load_output_index(const MapKeyType& key, MapType& map) const noexcept
        -> const Outpoints&;

    auto load_output(const block::Outpoint& id) noexcept(false)
        -> bitcoin::block::Output&;
    template <typename MapKeyType, typename MapType>
    auto load_output_index(const MapKeyType& key, MapType& map) noexcept
        -> Outpoints&;
    auto populate() noexcept -> void;
    auto write_output(
        const block::Outpoint& id,
        const bitcoin::block::Output& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::wallet
