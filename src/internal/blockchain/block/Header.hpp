// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/block/Header.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class BlockchainBlockHeader;
}  // namespace proto
// }  // namespace v1
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

    virtual auto EffectiveState() const noexcept -> Status = 0;
    virtual auto InheritedState() const noexcept -> Status = 0;
    virtual auto IsBlacklisted() const noexcept -> bool = 0;
    virtual auto IsDisconnected() const noexcept -> bool = 0;
    virtual auto LocalState() const noexcept -> Status = 0;
    virtual auto Serialize(SerializedType& out) const noexcept -> bool = 0;

    virtual auto CompareToCheckpoint(const block::Position& checkpoint) noexcept
        -> void = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual auto InheritHeight(const block::Header& parent) -> void = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual auto InheritState(const block::Header& parent) -> void = 0;
    virtual auto InheritWork(const blockchain::Work& parent) noexcept
        -> void = 0;
    virtual auto RemoveBlacklistState() noexcept -> void = 0;
    virtual auto RemoveCheckpointState() noexcept -> void = 0;
    virtual auto SetDisconnectedState() noexcept -> void = 0;

    virtual ~Header() = default;
};
}  // namespace opentxs::blockchain::block::internal
