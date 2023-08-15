// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Factory.hpp"  // IWYU pragma: associated

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "opentxs/blockchain/Category.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainBlock(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    alloc::Default alloc) noexcept -> blockchain::block::Block
{
    using enum blockchain::Category;

    switch (category(chain)) {
        case output_based: {

            return BitcoinBlock(crypto, chain, in, alloc);
        }
        case unknown_category:
        case balance_based:
        default: {
            LogError()("opentxs::factory::")(__func__)(": Unsupported type (")(
                print(chain))(")")
                .Flush();

            return {alloc};
        }
    }
}
}  // namespace opentxs::factory
