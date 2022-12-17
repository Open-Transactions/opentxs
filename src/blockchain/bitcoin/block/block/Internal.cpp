// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "internal/blockchain/bitcoin/block/Block.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::bitcoin::block::internal
{
auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}
}  // namespace opentxs::blockchain::bitcoin::block::internal
