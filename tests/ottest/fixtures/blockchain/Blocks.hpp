// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/Base.hpp"

namespace ottest
{
class OPENTXS_EXPORT BlockchainBlocks : virtual public Base
{
protected:
    auto CheckBlock(
        opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Hash& id,
        const opentxs::ReadView bytes) const noexcept -> bool;
    auto CheckGenesisBlock(opentxs::blockchain::Type chain) const noexcept
        -> bool;

    BlockchainBlocks() noexcept = default;

    ~BlockchainBlocks() override = default;
};
}  // namespace ottest
