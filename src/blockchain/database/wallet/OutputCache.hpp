// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <utility>

#include "blockchain/database/wallet/Position.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/wallet/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Output;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
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
class OutputCache
{
public:
    auto Exists(const block::Outpoint& id) const noexcept -> bool;
    auto Exists(const SubchainID& subchain, const block::Outpoint& id)
        const noexcept -> bool;
    auto GetAccount(const AccountID& id) const noexcept -> const Outpoints&;
    auto GetAssociation(
        const identifier::Generic& proposal,
        const block::Outpoint& output) const noexcept -> ProposalAssociation;
    auto GetConsumed(const identifier::Generic& proposal) const noexcept
        -> const Outpoints&;
    auto GetCreated(const identifier::Generic& proposal) const noexcept
        -> const Outpoints&;
    auto GetKey(const crypto::Key& id) const noexcept -> const Outpoints&;
    auto GetHeight() const noexcept -> block::Height;
    auto GetMatured(
        block::Height first,
        block::Height last,
        alloc::Strategy alloc) const noexcept -> Vector<block::Outpoint>;
    auto GetNym(const identifier::Nym& id) const noexcept -> const Outpoints&;
    auto GetNyms() const noexcept -> const Nyms&;
    auto GetOutput(const block::Outpoint& id) const noexcept(false)
        -> const protocol::bitcoin::base::block::Output&;
    auto GetPosition() const noexcept -> const db::Position&;
    auto GetPosition(const block::Position& id) const noexcept
        -> const Outpoints&;
    auto GetProposals(const block::Outpoint& id, alloc::Strategy alloc)
        const noexcept -> Vector<identifier::Generic>;
    auto GetReserved(alloc::Strategy alloc) const noexcept -> Reserved;
    auto GetReserved(const identifier::Generic& proposal, alloc::Strategy alloc)
        const noexcept -> Vector<UTXO>;
    auto GetState(const node::TxoState id) const noexcept -> const Outpoints&;
    auto GetSubchain(const SubchainID& id) const noexcept -> const Outpoints&;
    auto Print() const noexcept -> void;

    auto AddGenerationOutput(
        block::Height height,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto AddOutput(
        const block::Outpoint& id,
        storage::lmdb::Transaction& tx,
        protocol::bitcoin::base::block::Output output) noexcept -> bool;
    auto AddOutput(
        const block::Outpoint& id,
        const node::TxoState state,
        const block::Position& position,
        const AccountID& account,
        const SubchainID& subchain,
        storage::lmdb::Transaction& tx,
        protocol::bitcoin::base::block::Output output) noexcept -> bool;
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
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
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
    auto CheckProposals(
        const FlatSet<identifier::Generic>& proposals,
        alloc::Strategy alloc) noexcept(false) -> Vector<identifier::Generic>;
    auto ConsumeOutput(
        const Log& log,
        const identifier::Generic& proposal,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto CreateOutput(
        const Log& log,
        const identifier::Generic& proposal,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto DeleteGenerationAbove(
        block::Height good,
        OrphanedGeneration& out,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto FinishProposal(
        const Log& log,
        const identifier::Generic& proposal,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto GetOutput(const block::Outpoint& id) noexcept(false)
        -> protocol::bitcoin::base::block::Output&;
    auto GetOutput(
        const SubchainID& subchain,
        const block::Outpoint& id) noexcept(false)
        -> protocol::bitcoin::base::block::Output&;
    auto Populate() noexcept -> void;
    auto ProposalConfirmConsumed(
        const Log& log,
        const identifier::Generic& proposal,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto ProposalConfirmCreated(
        const Log& log,
        const identifier::Generic& proposal,
        const block::Outpoint& output,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto Release(
        const Log& log,
        const block::Outpoint& output,
        const identifier::Generic& proposal,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto UpdateOutput(
        const block::Outpoint& id,
        const protocol::bitcoin::base::block::Output& output,
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
    using Generation = Multimap<block::Height, block::Outpoint>;

    enum class Kind : bool { create, consume };

    static constexpr auto reserve_ = 10000_uz;
    static const Outpoints empty_outputs_;
    static const Nyms empty_nyms_;

    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    const blockchain::Type chain_;
    const block::Position& blank_;
    std::optional<db::Position> position_;
    MapType<block::Outpoint, protocol::bitcoin::base::block::Output> outputs_;
    MapType<identifier::Generic, Outpoints> accounts_;
    MapType<crypto::Key, Outpoints> keys_;
    MapType<identifier::Nym, Outpoints> nyms_;
    Nyms nym_list_;
    MapType<block::Position, Outpoints> positions_;
    MapType<node::TxoState, Outpoints> states_;
    MapType<identifier::Generic, Outpoints> subchains_;
    Multimap<block::Height, block::Outpoint> generation_outputs_;
    Multimap<block::Outpoint, identifier::Generic> output_to_proposal_;
    MapType<identifier::Generic, Outpoints> created_by_proposal_;
    MapType<identifier::Generic, Outpoints> consumed_by_proposal_;
    bool populated_;

    auto get_position() const noexcept -> const db::Position&;
    auto load_output(const block::Outpoint& id) const noexcept(false)
        -> const protocol::bitcoin::base::block::Output&;
    template <typename MapKeyType, typename MapType>
    auto load_output_index(const MapKeyType& key, MapType& map) const noexcept
        -> const Outpoints&;

    auto associate_proposal(
        const Log& log,
        const block::Outpoint& output,
        const identifier::Generic& proposal,
        Kind kind,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto dissociate_proposal(
        const Log& log,
        const block::Outpoint& output,
        const identifier::Generic& proposal,
        storage::lmdb::Transaction& tx) noexcept(false) -> void;
    auto is_finished(const identifier::Generic& id) noexcept -> bool;
    auto load_output(const block::Outpoint& id) noexcept(false)
        -> protocol::bitcoin::base::block::Output&
    {
        return const_cast<protocol::bitcoin::base::block::Output&>(
            std::as_const(*this).load_output(id));
    }
    template <typename MapKeyType, typename MapType>
    auto load_output_index(const MapKeyType& key, MapType& map) noexcept
        -> Outpoints&;
    auto write_output(
        const block::Outpoint& id,
        const protocol::bitcoin::base::block::Output& output,
        storage::lmdb::Transaction& tx) noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::wallet
