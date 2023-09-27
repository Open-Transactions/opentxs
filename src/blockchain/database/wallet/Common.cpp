// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/database/wallet/Types.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database::wallet
{
auto all_states() noexcept -> const States&
{
    using enum node::TxoState;
    static const auto data = States{
        UnconfirmedNew,
        UnconfirmedSpend,
        ConfirmedNew,
        ConfirmedSpend,
        OrphanedNew,
        OrphanedSpend,
        Immature,
    };

    return data;
}

auto is_mature(
    const block::Height minedAt,
    const block::Height bestChain,
    const block::Height target) noexcept -> bool
{
    return (bestChain - minedAt) >= target;
}
}  // namespace opentxs::blockchain::database::wallet
