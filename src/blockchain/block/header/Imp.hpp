// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/NumericHash.hpp"

#pragma once

#include <functional>

#include "blockchain/block/header/HeaderPrivate.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs::blockchain::block::implementation
{
class Header : virtual public HeaderPrivate
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> HeaderPrivate* override
    {
        return pmr::clone_as<HeaderPrivate>(this, {alloc});
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
    using internal::Header::Serialize;
    auto Serialize(SerializedType& out) const noexcept -> bool override;
    auto Type() const noexcept -> blockchain::Type final { return type_; }
    auto Valid() const noexcept -> bool final;
    auto Work() const noexcept -> blockchain::Work final;

    auto CompareToCheckpoint(const block::Position& checkpoint) noexcept
        -> void final;
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
    {
        return make_deleter(this);
    }
    auto InheritHeight(const block::Header& parent) -> void final;
    auto InheritState(const block::Header& parent) -> void final;
    auto InheritWork(const blockchain::Work& work) noexcept -> void final;
    auto RemoveBlacklistState() noexcept -> void final;
    auto RemoveCheckpointState() noexcept -> void final;
    auto SetDisconnectedState() noexcept -> void final;

    Header() = delete;
    Header(const Header& rhs, allocator_type alloc) noexcept;
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;

    ~Header() override;

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
        const blockchain::Work& inheritWork,
        allocator_type alloc) noexcept;

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
