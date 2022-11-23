// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "internal/blockchain/block/Parser.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <stdexcept>

#include "blockchain/bitcoin/block/parser/Base.hpp"
#include "blockchain/bitcoin/block/parser/Parser.hpp"
#include "blockchain/pkt/block/Parser.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
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

auto Parser::Check(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const ReadView bytes,
    Hash& out,
    ReadView& header) noexcept -> bool
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
            auto parser = bitcoin::block::Parser{crypto, type};

            if (parser(bytes, out)) {
                header = parser.GetHeader();

                return true;
            } else {

                return false;
            }
        }
        case PKT:
        case PKT_testnet: {
            auto parser = pkt::block::Parser{crypto, type};

            if (parser(bytes, out)) {
                header = parser.GetHeader();

                return true;
            } else {

                return false;
            }
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
    static const auto null = Hash{};

    return Construct(crypto, type, null, bytes, out);
}

auto Parser::Construct(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const Hash& expected,
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

            return bitcoin::block::Parser{crypto, type}(expected, bytes, out);
        }
        case PKT:
        case PKT_testnet: {

            return pkt::block::Parser{crypto, type}(expected, bytes, out);
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
    const Hash& expected,
    const network::zeromq::Message& message,
    std::shared_ptr<bitcoin::block::Block>& out) noexcept -> bool
{
    try {
        const auto bytes = [&]() -> ReadView {
            const auto body = message.Body();

            switch (body.size()) {
                case 3_uz: {

                    return body.at(2).Bytes();
                }
                case 4_uz: {

                    return {
                        reinterpret_cast<const char*>(
                            body.at(2).as<std::uintptr_t>()),
                        body.at(3).as<std::size_t>()};
                }
                default: {
                    throw std::runtime_error{"invalid message"};
                }
            }
        }();

        return Construct(crypto, type, expected, bytes, out);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_STATIC(Parser))(": ")(e.what()).Flush();

        return false;
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
