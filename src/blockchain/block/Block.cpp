// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/block/Block.hpp"             // IWYU pragma: associated
#include "internal/blockchain/block/Factory.hpp"  // IWYU pragma: associated

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainBlock(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in) noexcept -> std::shared_ptr<blockchain::block::Block>
{
    switch (chain) {
        case blockchain::Type::Bitcoin:
        case blockchain::Type::Bitcoin_testnet3:
        case blockchain::Type::BitcoinCash:
        case blockchain::Type::BitcoinCash_testnet3:
        case blockchain::Type::Litecoin:
        case blockchain::Type::Litecoin_testnet4:
        case blockchain::Type::BitcoinSV:
        case blockchain::Type::BitcoinSV_testnet3:
        case blockchain::Type::eCash:
        case blockchain::Type::eCash_testnet3:
        case blockchain::Type::PKT:
        case blockchain::Type::PKT_testnet:
        case blockchain::Type::UnitTest: {

            return BitcoinBlock(crypto, chain, in);
        }
        case blockchain::Type::Unknown:
        case blockchain::Type::Ethereum_frontier:
        case blockchain::Type::Ethereum_ropsten:
        default: {
            LogError()("opentxs::factory::")(__func__)(": Unsupported type (")(
                print(chain))(")")
                .Flush();

            return {};
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::implementation
{
Block::Block(const block::Header& header) noexcept
    : base_header_(header)
    , blank_bitcoin_(factory::BitcoinBlock())
{
    OT_ASSERT(blank_bitcoin_);
}

auto Block::asBitcoin() const noexcept -> const bitcoin::block::Block&
{
    return *blank_bitcoin_;
}

auto Block::asBitcoin() noexcept -> bitcoin::block::Block&
{
    return *blank_bitcoin_;
}

auto Block::Header() const noexcept -> const block::Header&
{
    return base_header_;
}

auto Block::ID() const noexcept -> const block::Hash&
{
    return base_header_.Hash();
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::block::implementation
