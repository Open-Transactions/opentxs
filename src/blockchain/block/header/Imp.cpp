// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/block/header/Imp.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>  // IWYU pragma: keep
#include <BlockchainBlockLocalData.pb.h>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/NumericHash.hpp"
#include "opentxs/blockchain/bitcoin/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::block::implementation
{
Header::Header(
    const api::Session& api,
    const VersionNumber version,
    const blockchain::Type type,
    block::Hash&& hash,
    block::Hash&& pow,
    block::Hash&& parentHash,
    const block::Height height,
    const Status status,
    const Status inheritStatus,
    const blockchain::Work& work,
    const blockchain::Work& inheritWork) noexcept
    : api_(api)
    , hash_(std::move(hash))
    , pow_(std::move(pow))
    , parent_hash_(std::move(parentHash))
    , type_(type)
    , version_(version)
    , work_(work)
    , height_(height)
    , status_(status)
    , inherit_status_(inheritStatus)
    , inherit_work_(inheritWork)
{
}

Header::Header(const Header& rhs) noexcept
    : api_(rhs.api_)
    , hash_(rhs.hash_)
    , pow_(rhs.pow_)
    , parent_hash_(rhs.parent_hash_)
    , type_(rhs.type_)
    , version_(rhs.version_)
    , work_(rhs.work_)
    , height_(rhs.height_)
    , status_(rhs.status_)
    , inherit_status_(rhs.inherit_status_)
    , inherit_work_(rhs.inherit_work_)
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

auto Header::minimum_work(const blockchain::Type chain) -> OTWork
{
    const auto maxTarget =
        OTNumericHash{factory::NumericHashNBits(NumericHash::MaxTarget(chain))};

    return OTWork{factory::Work(chain, maxTarget)};
}

auto Header::NumericHash() const noexcept -> OTNumericHash
{
    return OTNumericHash{factory::NumericHash(pow_)};
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
    local.set_work(work_->asHex());
    local.set_inherit_work(inherit_work_->asHex());

    return true;
}

auto Header::SetDisconnectedState() noexcept -> void
{
    status_ = Status::Disconnected;
    inherit_status_ = Status::Error;
}

auto Header::Valid() const noexcept -> bool { return NumericHash() < Target(); }

auto Header::Work() const noexcept -> OTWork
{
    if (parent_hash_.IsNull()) {
        return work_;
    } else {
        return work_ + inherit_work_;
    }
}
}  // namespace opentxs::blockchain::block::implementation
