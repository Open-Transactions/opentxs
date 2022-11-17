// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "internal/blockchain/block/Parser.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/parser/Base.hpp"
#include "blockchain/bitcoin/block/parser/Parser.hpp"
#include "blockchain/pkt/block/Parser.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::block
{
auto Parser::Check(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const Hash& expected,
    const ReadView bytes) noexcept -> bool
{
    using enum blockchain::Type;

    switch (type) {
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
        case UnitTest: {

            return bitcoin::block::Parser{crypto, type}(expected, bytes);
        }
        case PKT:
        case PKT_testnet: {

            return pkt::block::Parser{crypto, type}(expected, bytes);
        }
        case Unknown:
        case Ethereum_frontier:
        case Ethereum_ropsten:
        default: {
            LogError()(OT_PRETTY_STATIC(Parser))("unsupported chain: ")(
                print(type))
                .Flush();

            return false;
        }
    }
}

auto Parser::Construct(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const ReadView bytes,
    std::shared_ptr<blockchain::bitcoin::block::Block>& out) noexcept -> bool
{
    using enum blockchain::Type;

    switch (type) {
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
        case UnitTest: {

            return bitcoin::block::Parser{crypto, type}(bytes, out);
        }
        case PKT:
        case PKT_testnet: {

            return pkt::block::Parser{crypto, type}(bytes, out);
        }
        case Unknown:
        case Ethereum_frontier:
        case Ethereum_ropsten:
        default: {
            LogError()(OT_PRETTY_STATIC(Parser))("unsupported chain: ")(
                print(type))
                .Flush();

            return false;
        }
    }
}

auto Parser::Transaction(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const std::size_t position,
    const Time& time,
    const ReadView bytes,
    std::unique_ptr<bitcoin::block::internal::Transaction>& out) noexcept
    -> bool
{
    using enum blockchain::Type;

    switch (type) {
        case Bitcoin:
        case Bitcoin_testnet3:
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3:
        case UnitTest: {

            return bitcoin::block::Parser{crypto, type}(
                position, time, bytes, out);
        }
        case Unknown:
        case Ethereum_frontier:
        case Ethereum_ropsten:
        default: {
            LogError()(OT_PRETTY_STATIC(Parser))("unsupported chain: ")(
                print(type))
                .Flush();

            return false;
        }
    }
}
}  // namespace opentxs::blockchain::block
