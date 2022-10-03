// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/NumericHash.hpp"

#pragma once

#include <memory>

#include "blockchain/block/header/Header.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

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

class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class NumericHash;
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::implementation
{
class Header : virtual public block::Header::Imp
{
public:
    auto clone() const noexcept -> std::unique_ptr<Imp> override
    {
        return std::unique_ptr<Imp>(new Header(*this));
    }
    auto Difficulty() const noexcept -> blockchain::Work final { return work_; }
    auto EffectiveState() const noexcept -> Status final;
    auto Hash() const noexcept -> const block::Hash& final;
    auto Height() const noexcept -> block::Height final;
    auto IncrementalWork() const noexcept -> blockchain::Work final
    {
        return work_;
    }
    auto InheritedState() const noexcept -> Status final;
    auto IsBlacklisted() const noexcept -> bool final;
    auto IsDisconnected() const noexcept -> bool final;
    auto LocalState() const noexcept -> Status final;
    auto NumericHash() const noexcept -> block::NumericHash final;
    auto ParentHash() const noexcept -> const block::Hash& final;
    auto ParentWork() const noexcept -> blockchain::Work final
    {
        return inherit_work_;
    }
    auto Position() const noexcept -> block::Position final;
    using block::Header::Imp::Serialize;
    auto Serialize(SerializedType& out) const noexcept -> bool override;
    auto Type() const noexcept -> blockchain::Type final { return type_; }
    auto Valid() const noexcept -> bool final;
    auto Work() const noexcept -> blockchain::Work final;

    auto CompareToCheckpoint(const block::Position& checkpoint) noexcept
        -> void final;
    auto InheritHeight(const block::Header& parent) -> void final;
    auto InheritState(const block::Header& parent) -> void final;
    auto InheritWork(const blockchain::Work& work) noexcept -> void final;
    auto RemoveBlacklistState() noexcept -> void final;
    auto RemoveCheckpointState() noexcept -> void final;
    auto SetDisconnectedState() noexcept -> void final;

    Header() = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;

protected:
    static const VersionNumber default_version_{1};

    const block::Hash hash_;
    const ByteArray pow_;
    const block::Hash parent_hash_;
    const blockchain::Type type_;

    static auto minimum_work(const blockchain::Type chain) -> blockchain::Work;

    Header(
        const VersionNumber version,
        const blockchain::Type type,
        block::Hash&& hash,
        block::Hash&& pow,
        block::Hash&& parentHash,
        const block::Height height,
        const Status status,
        const Status inheritStatus,
        const blockchain::Work& work,
        const blockchain::Work& inheritWork) noexcept;
    Header(const Header& rhs) noexcept;

private:
    static const VersionNumber local_data_version_{1};

    const VersionNumber version_;
    const blockchain::Work work_;
    block::Height height_;
    Status status_;
    Status inherit_status_;
    blockchain::Work inherit_work_;
};
}  // namespace opentxs::blockchain::block::implementation
