// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/block/Block.hpp"

namespace opentxs::blockchain::bitcoin::block::internal
{
class Block : virtual public blockchain::block::internal::Block
{
public:
    static auto Blank() noexcept -> Block&;

    auto asBitcoin() const noexcept
        -> const bitcoin::block::internal::Block& final
    {
        return *this;
    }

    auto asBitcoin() noexcept -> bitcoin::block::internal::Block& final
    {
        return *this;
    }

    ~Block() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
