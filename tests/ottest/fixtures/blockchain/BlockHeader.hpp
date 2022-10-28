// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Test_BlockHeader : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    auto CheckState(const ot::blockchain::block::Header& header) const noexcept
        -> bool;
    auto GenesisHash(ot::blockchain::Type chain) const noexcept
        -> const ot::blockchain::block::Hash&;
    auto GenesisHeader(ot::blockchain::Type chain) const noexcept
        -> const ot::blockchain::block::Header&;
    auto IsEqual(
        const ot::blockchain::block::Header& lhs,
        const ot::blockchain::block::Header& rhs) const noexcept -> bool;

    Test_BlockHeader();
};
}  // namespace ottest
