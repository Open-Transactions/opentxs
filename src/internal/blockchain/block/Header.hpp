// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Header;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Position;
}  // namespace block

class Work;
}  // namespace blockchain

namespace proto
{
class BlockchainBlockHeader;
}  // namespace proto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
class Header
{
public:
    using SerializedType = proto::BlockchainBlockHeader;

    enum class Status : std::uint32_t {
        Error,
        Normal,
        Disconnected,
        CheckpointBanned,
        Checkpoint
    };

    virtual auto asBitcoin() const noexcept
        -> const blockchain::bitcoin::block::internal::Header&;
    virtual auto Difficulty() const noexcept -> blockchain::Work;
    virtual auto EffectiveState() const noexcept -> Status;
    virtual auto Hash() const noexcept -> const block::Hash&;
    virtual auto Height() const noexcept -> block::Height;
    virtual auto IncrementalWork() const noexcept -> blockchain::Work;
    virtual auto InheritedState() const noexcept -> Status;
    virtual auto IsBlacklisted() const noexcept -> bool;
    virtual auto IsDisconnected() const noexcept -> bool;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto LocalState() const noexcept -> Status;
    virtual auto NumericHash() const noexcept -> block::NumericHash;
    virtual auto ParentHash() const noexcept -> const block::Hash&;
    virtual auto ParentWork() const noexcept -> blockchain::Work;
    virtual auto Position() const noexcept -> block::Position;
    virtual auto Print() const noexcept -> UnallocatedCString;
    virtual auto Print(alloc::Default alloc) const noexcept -> CString;
    virtual auto Serialize(SerializedType& out) const noexcept -> bool;
    virtual auto Serialize(Writer&& destination, const bool bitcoinformat)
        const noexcept -> bool;
    virtual auto Target() const noexcept -> block::NumericHash;
    virtual auto Type() const noexcept -> blockchain::Type;
    virtual auto Valid() const noexcept -> bool;
    virtual auto Work() const noexcept -> blockchain::Work;

    virtual auto asBitcoin() noexcept
        -> blockchain::bitcoin::block::internal::Header&;
    virtual auto CompareToCheckpoint(const block::Position& checkpoint) noexcept
        -> void;
    /// Throws std::runtime_error if parent hash incorrect
    virtual auto InheritHeight(const block::Header& parent) -> void;
    /// Throws std::runtime_error if parent hash incorrect
    virtual auto InheritState(const block::Header& parent) -> void;
    virtual auto InheritWork(const blockchain::Work& parent) noexcept -> void;
    virtual auto RemoveBlacklistState() noexcept -> void;
    virtual auto RemoveCheckpointState() noexcept -> void;
    virtual auto SetDisconnectedState() noexcept -> void;

    virtual ~Header() = default;
};
}  // namespace opentxs::blockchain::block::internal
