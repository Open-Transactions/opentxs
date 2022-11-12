// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "internal/blockchain/block/Checker.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/Checker.hpp"
#include "blockchain/pkt/block/Checker.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs::blockchain::block
{
auto Checker::Check(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const Hash& expected,
    const ReadView bytes) noexcept -> bool
{
    switch (type) {
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Litecoin:
        case Type::Litecoin_testnet4:
        case Type::BitcoinSV:
        case Type::BitcoinSV_testnet3:
        case Type::eCash:
        case Type::eCash_testnet3:
        case Type::UnitTest: {
            auto checker = bitcoin::block::Checker{crypto, type};

            return checker(expected, bytes);
        }
        case Type::PKT:
        case Type::PKT_testnet: {
            auto checker = pkt::block::Checker{crypto, type};

            return checker(expected, bytes);
        }
        case Type::Unknown:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        default: {

            return false;
        }
    }
}
}  // namespace opentxs::blockchain::block
