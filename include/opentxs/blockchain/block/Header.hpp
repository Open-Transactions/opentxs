// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainBlockHeader;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Header
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

    virtual auto as_Bitcoin() const noexcept
        -> std::unique_ptr<bitcoin::Header> = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Header> = 0;
    virtual auto Difficulty() const noexcept -> OTWork = 0;
    virtual auto EffectiveState() const noexcept -> Status = 0;
    virtual auto Hash() const noexcept -> const block::Hash& = 0;
    virtual auto Height() const noexcept -> block::Height = 0;
    virtual auto IncrementalWork() const noexcept -> OTWork = 0;
    virtual auto InheritedState() const noexcept -> Status = 0;
    virtual auto IsBlacklisted() const noexcept -> bool = 0;
    virtual auto IsDisconnected() const noexcept -> bool = 0;
    virtual auto LocalState() const noexcept -> Status = 0;
    virtual auto NumericHash() const noexcept -> OTNumericHash = 0;
    virtual auto ParentHash() const noexcept -> const block::Hash& = 0;
    virtual auto ParentWork() const noexcept -> OTWork = 0;
    virtual auto Position() const noexcept -> block::Position = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(SerializedType& out) const noexcept
        -> bool = 0;
    virtual auto Serialize(
        const AllocateOutput destination,
        const bool bitcoinformat = true) const noexcept -> bool = 0;
    virtual auto Target() const noexcept -> OTNumericHash = 0;
    virtual auto Type() const noexcept -> blockchain::Type = 0;
    virtual auto Valid() const noexcept -> bool = 0;
    virtual auto Work() const noexcept -> OTWork = 0;

    virtual void CompareToCheckpoint(
        const block::Position& checkpoint) noexcept = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual void InheritHeight(const Header& parent) = 0;
    /// Throws std::runtime_error if parent hash incorrect
    virtual void InheritState(const Header& parent) = 0;
    virtual void InheritWork(const blockchain::Work& parent) noexcept = 0;
    virtual void RemoveBlacklistState() noexcept = 0;
    virtual void RemoveCheckpointState() noexcept = 0;
    virtual void SetDisconnectedState() noexcept = 0;

    virtual ~Header() = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::block
