// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/BlockHeader.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
Test_BlockHeader::Test_BlockHeader()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
{
}

auto Test_BlockHeader::CheckState(
    const ot::blockchain::block::Header& header) const noexcept -> bool
{
    const auto& internal = header.Internal();
    auto out{true};
    out &=
        (internal.EffectiveState() ==
         ot::blockchain::block::internal::Header::Status::Normal);
    out &=
        (internal.InheritedState() ==
         ot::blockchain::block::internal::Header::Status::Normal);
    out &=
        (internal.LocalState() ==
         ot::blockchain::block::internal::Header::Status::Checkpoint);
    out &= (false == internal.IsBlacklisted());
    out &= (false == internal.IsDisconnected());

    EXPECT_EQ(
        internal.EffectiveState(),
        ot::blockchain::block::internal::Header::Status::Normal);
    EXPECT_EQ(
        internal.InheritedState(),
        ot::blockchain::block::internal::Header::Status::Normal);
    EXPECT_EQ(
        internal.LocalState(),
        ot::blockchain::block::internal::Header::Status::Checkpoint);
    EXPECT_FALSE(internal.IsBlacklisted());
    EXPECT_FALSE(internal.IsDisconnected());

    return out;
}

auto Test_BlockHeader::GenesisHash(ot::blockchain::Type chain) const noexcept
    -> const ot::blockchain::block::Hash&
{
    return ot::blockchain::params::get(chain).GenesisHash();
}

auto Test_BlockHeader::GenesisHeader(ot::blockchain::Type chain) const noexcept
    -> const ot::blockchain::block::Header&
{
    return ot::blockchain::params::get(chain)
        .GenesisBlock(api_.Crypto())
        .Header();
}

auto Test_BlockHeader::IsEqual(
    const ot::blockchain::block::Header& lhs,
    const ot::blockchain::block::Header& rhs) const noexcept -> bool
{
    auto out{true};
    out &= (lhs.Difficulty() == rhs.Difficulty());
    out &= (lhs.Internal().EffectiveState() == rhs.Internal().EffectiveState());
    out &= (lhs.Hash() == rhs.Hash());
    out &= (lhs.Height() == rhs.Height());
    out &= (lhs.IncrementalWork() == rhs.IncrementalWork());
    out &= (lhs.Internal().InheritedState() == rhs.Internal().InheritedState());
    out &= (lhs.Internal().IsBlacklisted() == rhs.Internal().IsBlacklisted());
    out &= (lhs.Internal().IsDisconnected() == rhs.Internal().IsDisconnected());
    out &= (lhs.Internal().LocalState() == rhs.Internal().LocalState());
    out &= (lhs.NumericHash() == rhs.NumericHash());
    out &= (lhs.ParentHash() == rhs.ParentHash());
    out &= (lhs.ParentWork() == rhs.ParentWork());
    out &= (lhs.Position() == rhs.Position());
    out &= (lhs.Print() == rhs.Print());
    out &= (lhs.Target() == rhs.Target());
    out &= (lhs.Type() == rhs.Type());
    out &= (lhs.Valid() == rhs.Valid());
    out &= (lhs.Work() == rhs.Work());

    EXPECT_EQ(lhs.Difficulty(), rhs.Difficulty());
    EXPECT_EQ(lhs.Internal().EffectiveState(), rhs.Internal().EffectiveState());
    EXPECT_EQ(lhs.Hash(), rhs.Hash());
    EXPECT_EQ(lhs.Height(), rhs.Height());
    EXPECT_EQ(lhs.IncrementalWork(), rhs.IncrementalWork());
    EXPECT_EQ(lhs.Internal().InheritedState(), rhs.Internal().InheritedState());
    EXPECT_EQ(lhs.Internal().IsBlacklisted(), rhs.Internal().IsBlacklisted());
    EXPECT_EQ(lhs.Internal().IsDisconnected(), rhs.Internal().IsDisconnected());
    EXPECT_EQ(lhs.Internal().LocalState(), rhs.Internal().LocalState());
    EXPECT_EQ(lhs.NumericHash(), rhs.NumericHash());
    EXPECT_EQ(lhs.ParentHash(), rhs.ParentHash());
    EXPECT_EQ(lhs.ParentWork(), rhs.ParentWork());
    EXPECT_EQ(lhs.Position(), rhs.Position());
    EXPECT_EQ(lhs.Print(), rhs.Print());
    EXPECT_EQ(lhs.Target(), rhs.Target());
    EXPECT_EQ(lhs.Type(), rhs.Type());
    EXPECT_EQ(lhs.Valid(), rhs.Valid());
    EXPECT_EQ(lhs.Work(), rhs.Work());

    return out;
}
}  // namespace ottest
