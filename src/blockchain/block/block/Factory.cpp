// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Factory.hpp"  // IWYU pragma: associated

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
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
    using enum blockchain::Type;

    switch (chain) {
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case Litecoin:
        case Litecoin_testnet4:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case PKT:
        case PKT_testnet:
        case UnitTest: {

            return BitcoinBlock(crypto, chain, in, alloc);
        }
        case Unknown:
        case Ethereum_frontier:
        case Ethereum_ropsten:
        default: {
            LogError()("opentxs::factory::")(__func__)(": Unsupported type (")(
                print(chain))(")")
                .Flush();

            return {alloc};
        }
    }
}
}  // namespace opentxs::factory