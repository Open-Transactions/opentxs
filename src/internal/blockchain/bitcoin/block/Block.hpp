// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/blockchain/bitcoin/block/Block.hpp"

namespace opentxs::blockchain::bitcoin::block::internal
{
class Block : virtual public bitcoin::block::Block
{
public:
    virtual auto clone_bitcoin() const noexcept -> std::unique_ptr<Block> = 0;
    auto InternalBitcoin() const noexcept -> const internal::Block& final
    {
        return *this;
    }

    auto InternalBitcoin() noexcept -> internal::Block& final { return *this; }

    ~Block() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
