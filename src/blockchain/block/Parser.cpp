// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Parser.hpp"  // IWYU pragma: associated

#include <span>
#include <stdexcept>

#include "blockchain/protocol/bitcoin/base/block/parser/Base.hpp"
#include "blockchain/protocol/bitcoin/base/block/parser/Parser.hpp"
#include "blockchain/protocol/bitcoin/pkt/block/Parser.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Category.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::block
{
auto Parser::Check(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const Hash& expected,
    const ReadView bytes,
    alloc::Strategy alloc) noexcept -> bool
{
    using enum blockchain::Category;
    using enum blockchain::Type;

    switch (category(type)) {
        case output_based: {
            if (PKT == associated_mainnet(type)) {

                return protocol::bitcoin::pkt::block::Parser{
                    crypto, type, alloc}(expected, bytes);
            } else {

                return protocol::bitcoin::base::block::Parser{
                    crypto, type, alloc}(expected, bytes);
            }
        }
        case unknown_category:
        case balance_based:
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
    blockchain::block::Block& out,
    alloc::Strategy alloc) noexcept -> bool
{
    static const auto null = Hash{};

    return Construct(crypto, type, null, bytes, out, alloc);
}

auto Parser::Construct(
    const api::Crypto& crypto,
    const blockchain::Type type,
    const Hash& expected,
    const ReadView bytes,
    blockchain::block::Block& out,
    alloc::Strategy alloc) noexcept -> bool
{
    using enum blockchain::Category;
    using enum blockchain::Type;

    switch (category(type)) {
        case output_based: {
            if (PKT == associated_mainnet(type)) {

                return protocol::bitcoin::pkt::block::Parser{
                    crypto, type, alloc}(expected, bytes, out);
            } else {

                return protocol::bitcoin::base::block::Parser{
                    crypto, type, alloc}(expected, bytes, out);
            }
        }
        case unknown_category:
        case balance_based:
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
    const network::zeromq::Message& message,
    Vector<Block>& out,
    alloc::Strategy alloc) noexcept -> bool
{
    using namespace node::blockoracle;

    try {
        const auto body = message.Payload();
        const auto count = body.size();

        if ((3_uz > count) || (0_uz == count % 2_uz)) {
            const auto error =
                UnallocatedCString{"invalid message frame count: "}.append(
                    std::to_string(count));

            throw std::runtime_error{error};
        }

        const auto blocks = (count - 1_uz) / 2_uz;
        out.reserve(blocks);
        out.clear();

        for (auto n = 1_uz; n < count; n += 2_uz) {
            const auto hash = block::Hash{body[n].Bytes()};
            const auto& data = body[n + 1_uz];
            const auto location = parse_block_location(data);
            const auto bytes = reader(location, alloc.work_);
            auto& block = out.emplace_back();

            if (false == Construct(crypto, type, hash, bytes, block, alloc)) {
                out.pop_back();
                const auto error =
                    UnallocatedCString{"failed to construct block "}.append(
                        hash.asHex());

                throw std::runtime_error{error};
            }
        }

        return true;
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
    block::Transaction& out,
    alloc::Strategy alloc) noexcept -> bool
{
    using enum blockchain::Category;

    switch (category(type)) {
        case output_based: {

            return protocol::bitcoin::base::block::Parser{crypto, type, alloc}(
                position, time, bytes, out);
        }
        case unknown_category:
        case balance_based:
        default: {
            LogError()(OT_PRETTY_STATIC(Parser))("unsupported chain: ")(
                print(type))
                .Flush();

            return false;
        }
    }
}
}  // namespace opentxs::blockchain::block
