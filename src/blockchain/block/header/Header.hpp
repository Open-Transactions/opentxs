// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <memory>

#include "internal/blockchain/bitcoin/block/Header.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

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
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class NumericHash;
class Position;
}  // namespace block

class Work;
}  // namespace blockchain

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class Header::Imp : virtual public internal::Header
{
public:
    auto as_Bitcoin() const noexcept
        -> const blockchain::bitcoin::block::internal::Header& override
    {
        return blockchain::bitcoin::block::internal::Header::Blank();
    }
    virtual auto clone() const noexcept -> std::unique_ptr<Imp>
    {
        return std::make_unique<Imp>();
    }
    virtual auto Difficulty() const noexcept -> blockchain::Work;
    auto EffectiveState() const noexcept -> Status override { return {}; }
    virtual auto Hash() const noexcept -> const block::Hash&;
    virtual auto Height() const noexcept -> block::Height { return {}; }
    virtual auto IncrementalWork() const noexcept -> blockchain::Work;
    auto InheritedState() const noexcept -> Status override { return {}; }
    auto IsBlacklisted() const noexcept -> bool override { return {}; }
    auto IsDisconnected() const noexcept -> bool override { return {}; }
    auto LocalState() const noexcept -> Status override { return {}; }
    virtual auto NumericHash() const noexcept -> block::NumericHash;
    virtual auto ParentHash() const noexcept -> const block::Hash&;
    virtual auto ParentWork() const noexcept -> blockchain::Work;
    virtual auto Position() const noexcept -> block::Position { return {}; }
    virtual auto Print() const noexcept -> UnallocatedCString { return {}; }
    using internal::Header::Serialize;
    virtual auto Serialize(Writer&& destination, const bool bitcoinformat)
        const noexcept -> bool
    {
        return {};
    }
    auto Serialize(SerializedType& out) const noexcept -> bool override
    {
        return {};
    }
    virtual auto Target() const noexcept -> block::NumericHash;
    virtual auto Type() const noexcept -> blockchain::Type { return {}; }
    virtual auto Valid() const noexcept -> bool { return {}; }
    virtual auto Work() const noexcept -> blockchain::Work;

    auto as_Bitcoin() noexcept
        -> blockchain::bitcoin::block::internal::Header& override
    {
        return blockchain::bitcoin::block::internal::Header::Blank();
    }
    auto CompareToCheckpoint(const block::Position& checkpoint) noexcept
        -> void override
    {
    }
    auto InheritHeight(const block::Header& parent) -> void override {}
    auto InheritState(const block::Header& parent) -> void override {}
    auto InheritWork(const blockchain::Work& work) noexcept -> void override {}
    auto RemoveBlacklistState() noexcept -> void override {}
    auto RemoveCheckpointState() noexcept -> void override {}
    auto SetDisconnectedState() noexcept -> void override {}

    Imp() = default;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override = default;
};
}  // namespace opentxs::blockchain::block
