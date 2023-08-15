// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Block.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
auto Block::Blank() noexcept -> Block&
{
    static auto blank = Block{};

    return blank;
}
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
