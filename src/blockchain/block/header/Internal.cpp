// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Header.hpp"  // IWYU pragma: associated

#include "internal/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/NumericHash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::block::internal
{
auto Header::asBitcoin() const noexcept
    -> const bitcoin::block::internal::Header&
{
    return bitcoin::block::internal::Header::Blank();
}

auto Header::asBitcoin() noexcept -> bitcoin::block::internal::Header&
{
    return bitcoin::block::internal::Header::Blank();
}

auto Header::CompareToCheckpoint(const block::Position&) noexcept -> void {}

auto Header::Difficulty() const noexcept -> blockchain::Work { return {}; }

auto Header::EffectiveState() const noexcept -> Status { return {}; }

auto Header::Hash() const noexcept -> const block::Hash&
{
    static const auto blank = block::Hash{};

    return blank;
}

auto Header::Height() const noexcept -> block::Height { return {}; }

auto Header::IncrementalWork() const noexcept -> blockchain::Work { return {}; }

auto Header::InheritHeight(const block::Header& parent) -> void {}

auto Header::InheritState(const block::Header& parent) -> void {}

auto Header::InheritWork(const blockchain::Work& parent) noexcept -> void {}

auto Header::InheritedState() const noexcept -> Status { return {}; }

auto Header::IsBlacklisted() const noexcept -> bool { return {}; }

auto Header::IsDisconnected() const noexcept -> bool { return {}; }

auto Header::IsValid() const noexcept -> bool { return {}; }

auto Header::LocalState() const noexcept -> Status { return {}; }

auto Header::NumericHash() const noexcept -> block::NumericHash { return {}; }

auto Header::ParentHash() const noexcept -> const block::Hash&
{
    return Hash();
}

auto Header::ParentWork() const noexcept -> blockchain::Work { return {}; }

auto Header::Position() const noexcept -> block::Position { return {}; }

auto Header::Print() const noexcept -> UnallocatedCString { return {}; }

auto Header::Print(alloc::Strategy alloc) const noexcept -> CString
{
    return CString{alloc.result_};
}

auto Header::RemoveBlacklistState() noexcept -> void {}

auto Header::RemoveCheckpointState() noexcept -> void {}

auto Header::Serialize(SerializedType&) const noexcept -> bool { return {}; }

auto Header::Serialize(Writer&&, const bool) const noexcept -> bool
{
    return {};
}

auto Header::SetDisconnectedState() noexcept -> void {}

auto Header::Target() const noexcept -> block::NumericHash { return {}; }

auto Header::Type() const noexcept -> blockchain::Type { return {}; }

auto Header::Valid() const noexcept -> bool { return {}; }

auto Header::Work() const noexcept -> blockchain::Work { return {}; }
}  // namespace opentxs::blockchain::block::internal
