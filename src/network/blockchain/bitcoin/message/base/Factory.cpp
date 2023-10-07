// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/network/blockchain/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Header.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/addr/Imp.hpp"
#include "network/blockchain/bitcoin/message/addr2/Imp.hpp"
#include "network/blockchain/bitcoin/message/base/Imp.hpp"
#include "network/blockchain/bitcoin/message/block/Imp.hpp"
#include "network/blockchain/bitcoin/message/cfcheckpt/Imp.hpp"
#include "network/blockchain/bitcoin/message/cfheaders/Imp.hpp"
#include "network/blockchain/bitcoin/message/cfilter/Imp.hpp"
#include "network/blockchain/bitcoin/message/getaddr/Imp.hpp"
#include "network/blockchain/bitcoin/message/getblocks/Imp.hpp"
#include "network/blockchain/bitcoin/message/getcfcheckpt/Imp.hpp"
#include "network/blockchain/bitcoin/message/getcfheaders/Imp.hpp"
#include "network/blockchain/bitcoin/message/getcfilters/Imp.hpp"
#include "network/blockchain/bitcoin/message/getdata/Imp.hpp"
#include "network/blockchain/bitcoin/message/getheaders/Imp.hpp"
#include "network/blockchain/bitcoin/message/headers/Imp.hpp"
#include "network/blockchain/bitcoin/message/inv/Imp.hpp"
#include "network/blockchain/bitcoin/message/mempool/Imp.hpp"
#include "network/blockchain/bitcoin/message/notfound/Imp.hpp"
#include "network/blockchain/bitcoin/message/ping/Imp.hpp"
#include "network/blockchain/bitcoin/message/pong/Imp.hpp"
#include "network/blockchain/bitcoin/message/reject/Imp.hpp"
#include "network/blockchain/bitcoin/message/sendaddr2/Imp.hpp"
#include "network/blockchain/bitcoin/message/tx/Imp.hpp"
#include "network/blockchain/bitcoin/message/verack/Imp.hpp"
#include "network/blockchain/bitcoin/message/version/Imp.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
using namespace std::literals;

auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    network::zeromq::Message&& incoming,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message
{
    const auto payload = incoming.Payload();
    const auto frames = payload.size();

    switch (payload.size()) {
        case 0:
        case 1:
        case 2:
        case 4: {
            LogError()()("invalid ")(print(chain))(" message (")(
                frames)(" payload frames)")
                .Flush();

            return {alloc};
        }
        case 3: {

            return BitcoinP2PMessage(
                api,
                chain,
                type,
                version,
                payload[1].Bytes(),
                payload[2].Bytes(),
                alloc);
        }
        default: {
            return BitcoinP2PMessageZMQ(
                api, chain, type, version, std::move(incoming), alloc);
        }
    }
}

template <typename ReturnType, typename... Args>
auto bitcoin_p2p_builder(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::Command command,
    alloc::Default alloc,
    std::optional<ByteArray> checksum,
    ReadView payload,
    Args... args) noexcept(false)
    -> network::blockchain::bitcoin::message::internal::Message
{
    auto out = pmr::construct<ReturnType>(
        alloc, api, chain, std::move(checksum), payload, args...);
    check_finished_nonfatal(payload, out->Describe());

    return out;
}

auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    ReadView headerBytes,
    ReadView payloadBytes,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message
{
    try {
        using namespace network::blockchain::bitcoin::message;
        using network::blockchain::bitcoin::message::internal::Header;
        const auto header = Header{headerBytes};

        if (false == header.IsValid(chain)) {

            throw std::runtime_error{"invalid header"};
        }

        if (false == header.Verify(api, chain, payloadBytes)) {

            throw std::runtime_error{
                "checksum failure for "s.append(print(chain))
                    .append(" ")
                    .append(print(header.Command()))};
        }

        return BitcoinP2PMessage(
            api,
            chain,
            type,
            version,
            header.Command(),
            header.Describe(),
            std::optional<ByteArray>{header.Checksum()},
            payloadBytes,
            alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc};
    }
}

auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    const network::blockchain::bitcoin::message::Command command,
    const std::string_view commandText,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message
{
    using namespace network::blockchain::bitcoin::message;

    const auto blank = [&]() {
        using BlankType =
            network::blockchain::bitcoin::message::implementation::Message;

        if (Command::unknown == command) {

            return pmr::construct<BlankType>(
                alloc, api, chain, commandText, std::move(checksum));
        } else {

            return pmr::construct<BlankType>(
                alloc, api, chain, command, std::move(checksum));
        }
    };

    try {
        switch (command) {
            case Command::addr: {
                return bitcoin_p2p_builder<addr::Message>(
                    api,
                    chain,
                    command,
                    alloc,
                    std::move(checksum),
                    payload,
                    version);
            }
            case Command::addr2: {
                return bitcoin_p2p_builder<addr2::Message>(
                    api,
                    chain,
                    command,
                    alloc,
                    std::move(checksum),
                    payload,
                    version);
            }
            case Command::block: {
                return bitcoin_p2p_builder<block::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::cfcheckpt: {
                return bitcoin_p2p_builder<cfcheckpt::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::cfheaders: {
                return bitcoin_p2p_builder<cfheaders::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::cfilter: {
                return bitcoin_p2p_builder<cfilter::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getaddr: {
                return bitcoin_p2p_builder<getaddr::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getblocks: {
                return bitcoin_p2p_builder<getblocks::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getcfcheckpt: {
                return bitcoin_p2p_builder<getcfcheckpt::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getcfheaders: {
                return bitcoin_p2p_builder<getcfheaders::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getcfilters: {
                return bitcoin_p2p_builder<getcfilters::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getdata: {
                return bitcoin_p2p_builder<getdata::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::getheaders: {
                return bitcoin_p2p_builder<getheaders::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::headers: {
                return bitcoin_p2p_builder<headers::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::inv: {
                return bitcoin_p2p_builder<inv::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::mempool: {
                return bitcoin_p2p_builder<mempool::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::notfound: {
                return bitcoin_p2p_builder<notfound::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::ping: {
                return bitcoin_p2p_builder<ping::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::pong: {
                return bitcoin_p2p_builder<pong::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::reject: {
                return bitcoin_p2p_builder<reject::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::sendaddr2: {
                return bitcoin_p2p_builder<sendaddr2::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::tx: {
                return bitcoin_p2p_builder<tx::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::verack: {
                return bitcoin_p2p_builder<verack::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::version: {
                return bitcoin_p2p_builder<version::Message>(
                    api, chain, command, alloc, std::move(checksum), payload);
            }
            case Command::unknown:
            case Command::alert:
            case Command::authch:
            case Command::avahello:
            case Command::blocktxn:
            case Command::checkorder:
            case Command::cmpctblock:
            case Command::feefilter:
            case Command::filteradd:
            case Command::filterclear:
            case Command::filterload:
            case Command::getblocktxn:
            case Command::merkleblock:
            case Command::protoconf:
            case Command::reply:
            case Command::sendcmpct:
            case Command::senddsq:
            case Command::sendheaders2:
            case Command::sendheaders:
            case Command::submitorder:
            case Command::xversion:
            default: {

                return blank();
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what())(" while processing ")(print(chain))(" ")(
            commandText)
            .Flush();

        return blank();
    }
}

auto BitcoinP2PMessageZMQ(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    network::zeromq::Message&& incoming,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message
{
    try {
        using namespace network::blockchain;
        using namespace network::blockchain::bitcoin::message;

        if (const auto val = decode(incoming); val != chain) {

            throw std::runtime_error{
                "message is encoded for "s.append(print(val))
                    .append(" but was received by ")
                    .append(print(chain))};
        }

        const auto data = incoming.Payload();

        assert_true(data.size() >= 5_uz);

        const auto command = data[3].Bytes();
        constexpr auto maxCommand = 16_uz;

        if (command.empty()) { throw std::runtime_error{"missing command"}; }

        if (const auto size = command.size(); maxCommand < size) {

            throw std::runtime_error{
                "command length of "s.append(std::to_string(size))
                    .append(" exceeds maximum value of ")
                    .append(std::to_string(maxCommand))
                    .append(" for ")
                    .append(print(chain))};
        }

        auto payload = data[4].Bytes();

        return BitcoinP2PMessage(
            api,
            chain,
            type,
            version,
            GetCommand(command),
            command,
            std::nullopt,
            payload,
            alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
