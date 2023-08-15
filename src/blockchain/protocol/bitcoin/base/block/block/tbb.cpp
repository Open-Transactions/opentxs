// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/block/Imp.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>

#include "TBB.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
auto Block::calculate_transaction_sizes() const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const blockchain::block::Transaction*>;
    const auto& data = transactions_;

    return tbb::parallel_reduce(
        Range{data.data(), std::next(data.data(), data.size())},
        0_uz,
        [](const Range& r, std::size_t init) {
            for (const auto& i : r) {
                init += i.Internal().asBitcoin().CalculateSize();
            }

            return init;
        },
        [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
