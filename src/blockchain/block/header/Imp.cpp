// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/header/Imp.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainBlockHeader.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/BlockchainBlockLocalData.pb.h>
#include <compare>
#include <cstdint>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs::blockchain::block::implementation
{
Header::Header(
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
    allocator_type alloc) noexcept
    : HeaderPrivate(alloc)
    , hash_(std::move(hash))
    , pow_(std::move(pow), alloc)
    , parent_hash_(std::move(parentHash))
    , type_(type)
    , version_(version)
    , work_(work, alloc)
    , height_(height)
    , status_(status)
    , inherit_status_(inheritStatus)
    , inherit_work_(inheritWork, alloc)
{
}

Header::Header(const Header& rhs, allocator_type alloc) noexcept
    : HeaderPrivate(rhs, alloc)
    , hash_(rhs.hash_)
    , pow_(rhs.pow_, alloc)
    , parent_hash_(rhs.parent_hash_)
    , type_(rhs.type_)
    , version_(rhs.version_)
    , work_(rhs.work_, alloc)
    , height_(rhs.height_)
    , status_(rhs.status_)
    , inherit_status_(rhs.inherit_status_)
    , inherit_work_(rhs.inherit_work_, alloc)
{
}

auto Header::EffectiveState() const noexcept -> Header::Status
{
    if (Status::CheckpointBanned == inherit_status_) { return inherit_status_; }

    if (Status::Disconnected == inherit_status_) { return inherit_status_; }

    if (Status::Checkpoint == status_) { return Status::Normal; }

    return status_;
}

auto Header::CompareToCheckpoint(const block::Position& checkpoint) noexcept
    -> void
{
    const auto& [height, hash] = checkpoint;

    if (height == height_) {
        if (hash == hash_) {
            status_ = Status::Checkpoint;
        } else {
            status_ = Status::CheckpointBanned;
        }
    } else {
        status_ = Header::Status::Normal;
    }
}

auto Header::Hash() const noexcept -> const block::Hash& { return hash_; }

auto Header::Height() const noexcept -> block::Height { return height_; }

auto Header::InheritedState() const noexcept -> Header::Status
{
    return inherit_status_;
}

auto Header::InheritHeight(const block::Header& parent) -> void
{
    if (parent.Hash() != parent_hash_) {
        throw std::runtime_error("Invalid parent");
    }

    height_ = parent.Height() + 1;
}

auto Header::InheritState(const block::Header& parent) -> void
{
    if (parent.Hash() != parent_hash_) {
        throw std::runtime_error("Invalid parent");
    }

    inherit_status_ = parent.Internal().EffectiveState();
}

auto Header::InheritWork(const blockchain::Work& work) noexcept -> void
{
    inherit_work_ = work;
}

auto Header::IsDisconnected() const noexcept -> bool
{
    return Status::Disconnected == EffectiveState();
}

auto Header::IsBlacklisted() const noexcept -> bool
{
    return Status::CheckpointBanned == EffectiveState();
}

auto Header::LocalState() const noexcept -> Header::Status { return status_; }

auto Header::minimum_work(const blockchain::Type chain) -> blockchain::Work
{
    const auto maxTarget = block::NumericHash{params::get(chain).Difficulty()};

    return {maxTarget, chain};
}

auto Header::NumericHash() const noexcept -> block::NumericHash
{
    return {pow_};
}

auto Header::ParentHash() const noexcept -> const block::Hash&
{
    return parent_hash_;
}

auto Header::Position() const noexcept -> block::Position
{
    return {height_, hash_};
}

auto Header::RemoveBlacklistState() noexcept -> void
{
    status_ = Status::Normal;
    inherit_status_ = Status::Normal;
}

auto Header::RemoveCheckpointState() noexcept -> void
{
    status_ = Status::Normal;
}

auto Header::Serialize(SerializedType& output) const noexcept -> bool
{
    output.set_version(version_);
    output.set_type(static_cast<std::uint32_t>(type_));
    auto& local = *output.mutable_local();
    local.set_version(local_data_version_);
    local.set_height(height_);
    local.set_status(static_cast<std::uint32_t>(status_));
    local.set_inherit_status(static_cast<std::uint32_t>(inherit_status_));
    local.set_work(work_.asHex());
    local.set_inherit_work(inherit_work_.asHex());

    return true;
}

auto Header::SetDisconnectedState() noexcept -> void
{
    status_ = Status::Disconnected;
    inherit_status_ = Status::Error;
}

auto Header::SetHeight(block::Height height) noexcept -> void
{
    height_ = height;
}

auto Header::Valid() const noexcept -> bool { return NumericHash() < Target(); }

auto Header::Work() const noexcept -> blockchain::Work
{
    if (parent_hash_.IsNull()) {
        return work_;
    } else {
        return work_ + inherit_work_;
    }
}

Header::~Header() = default;
}  // namespace opentxs::blockchain::block::implementation
