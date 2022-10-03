// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <optional>

#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Header;
class Position;
}  // namespace block

namespace database
{
class Header;
}  // namespace database
}  // namespace blockchain

class Factory;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class UpdateTransaction
{
public:
    auto BestChain() const -> const database::BestHashes& { return best_; }
    auto Checkpoint() const -> block::Position;
    auto Connected() const -> const database::Segments& { return connect_; }
    auto Disconnected() const -> const database::Segments&
    {
        return disconnected_;
    }
    auto EffectiveBestBlock(const block::Height height) const noexcept(false)
        -> block::Hash;
    auto EffectiveCheckpoint() const noexcept -> bool;
    auto EffectiveDisconnectedHashes() const noexcept
        -> const database::DisconnectedList&
    {
        return disconnected();
    }
    auto EffectiveHasDisconnectedChildren(
        const block::Hash& hash) const noexcept -> bool;
    auto EffectiveHeaderExists(const block::Hash& hash) const noexcept -> bool;
    auto EffectiveIsSibling(const block::Hash& hash) const noexcept -> bool
    {
        return 0 < siblings().count(hash);
    }
    auto EffectiveSiblingHashes() const noexcept -> const database::Hashes&
    {
        return siblings();
    }
    auto HaveCheckpoint() const -> bool { return have_checkpoint_; }
    auto HaveReorg() const -> bool { return have_reorg_; }
    auto ReorgParent() const -> const block::Position& { return reorg_from_; }
    auto SiblingsToAdd() const -> const database::Hashes& { return add_sib_; }
    auto SiblingsToDelete() const -> const database::Hashes&
    {
        return delete_sib_;
    }
    auto UpdatedHeaders() const -> const database::UpdatedHeader&
    {
        return headers_;
    }

    void AddSibling(const block::Position& position);
    void AddToBestChain(const block::Position& position);
    void ClearCheckpoint();
    void ConnectBlock(database::ChainSegment&& segment);
    void DisconnectBlock(const block::Header& header);
    // throws std::out_of_range if header does not exist
    auto Header(const block::Hash& hash) noexcept(false) -> block::Header&;
    void RemoveSibling(const block::Hash& hash);
    void SetCheckpoint(block::Position&& checkpoint);
    void SetReorgParent(const block::Position& pos) noexcept;
    // Stages best block for possible metadata update
    auto Stage() noexcept -> block::Header&;
    // Stages a brand new header
    auto Stage(std::unique_ptr<block::Header> header) noexcept
        -> block::Header&;
    // Stages an existing header for possible metadata update
    auto Stage(const block::Hash& hash) noexcept(false) -> block::Header&;
    // Stages an existing header for possible metadata update
    auto Stage(const block::Height& height) noexcept(false) -> block::Header&;

    UpdateTransaction(const api::Session& api, const database::Header& db);

private:
    friend opentxs::Factory;

    const api::Session& api_;
    const database::Header& db_;
    bool have_reorg_;
    bool have_checkpoint_;
    block::Position reorg_from_;
    block::Position checkpoint_;
    database::UpdatedHeader headers_;
    database::BestHashes best_;
    database::Hashes add_sib_;
    database::Hashes delete_sib_;
    database::Segments connect_;
    database::Segments disconnected_;
    mutable std::optional<database::DisconnectedList> cached_disconnected_;
    mutable std::optional<database::Hashes> cached_siblings_;

    auto disconnected() const noexcept -> database::DisconnectedList&;
    auto siblings() const noexcept -> database::Hashes&;
    auto stage(
        const bool newHeader,
        std::unique_ptr<block::Header> header) noexcept -> block::Header&;
};
}  // namespace opentxs::blockchain::node
