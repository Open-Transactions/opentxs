// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/block/Imp.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <numeric>

#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/util/Container.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunneeded-member-function"
#pragma GCC diagnostic ignored "-Wunused-member-function"
namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
auto Block::calculate_transaction_sizes() const noexcept -> std::size_t
{
    struct Visitor {
        auto operator()(
            std::size_t lhs,
            const blockchain::block::Transaction& rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs.Internal().asBitcoin().CalculateSize();
        }
        auto operator()(
            const blockchain::block::Transaction& lhs,
            std::size_t rhs) const noexcept -> std::size_t
        {
            return lhs.Internal().asBitcoin().CalculateSize() + rhs;
        }
        auto operator()(
            const blockchain::block::Transaction& lhs,
            const blockchain::block::Transaction& rhs) const noexcept
            -> std::size_t
        {
            return lhs.Internal().asBitcoin().CalculateSize() +
                   rhs.Internal().asBitcoin().CalculateSize();
        }
        auto operator()(std::size_t lhs, std::size_t rhs) const noexcept
            -> std::size_t
        {
            return lhs + rhs;
        }
    };

    return std::reduce(
        transactions_.begin(), transactions_.end(), 0_uz, Visitor{});
}
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
#pragma GCC diagnostic pop
