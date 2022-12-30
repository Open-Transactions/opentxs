// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Header.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
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
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    network::zeromq::Message&& incoming,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message
{
    auto header = ReadView{};
    auto payload = ReadView{};
    const auto body = incoming.Body();
    const auto frames = body.size();

    if (1_uz < frames) { header = body.at(1).Bytes(); }

    if (2_uz < frames) { payload = body.at(2).Bytes(); }

    return BitcoinP2PMessage(api, chain, type, version, header, payload, alloc);
}

template <typename ReturnType, typename... Args>
auto bitcoin_p2p_builder(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::Command command,
    std::optional<ByteArray> checksum,
    ReadView payload,
    alloc::Default alloc,
    Args... args) noexcept(false)
    -> network::blockchain::bitcoin::message::internal::Message
{
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = nullptr;

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(out, api, chain, std::move(checksum), payload, args...);
        check_finished_nonfatal(payload, out->Describe());

        return out;
    } catch (...) {
        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        std::rethrow_exception(std::current_exception());
    }
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
            const auto error =
                UnallocatedCString{"checksum failure for "}.append(
                    print(header.Command()));

            throw std::runtime_error{error};
        }

        auto checksum = std::optional<ByteArray>{header.Checksum()};
        const auto command = header.Command();
        const auto blank = [&]() {
            using BlankType =
                network::blockchain::bitcoin::message::implementation::Message;
            auto pmr = alloc::PMR<BlankType>{alloc};
            BlankType* out = nullptr;

            try {
                out = pmr.allocate(1_uz);

                OT_ASSERT(nullptr != out);

                if (Command::unknown == command) {
                    pmr.construct(
                        out,
                        api,
                        chain,
                        header.Describe(),
                        std::move(checksum));
                } else {
                    pmr.construct(
                        out, api, chain, command, std::move(checksum));
                }

                return out;
            } catch (...) {
                if (nullptr != out) { pmr.deallocate(out, 1_uz); }

                std::rethrow_exception(std::current_exception());
            }
        };

        try {
            switch (command) {
                case Command::addr: {
                    return bitcoin_p2p_builder<addr::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc,
                        version);
                }
                case Command::addr2: {
                    return bitcoin_p2p_builder<addr2::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc,
                        version);
                }
                case Command::block: {
                    return bitcoin_p2p_builder<block::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::cfcheckpt: {
                    return bitcoin_p2p_builder<cfcheckpt::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::cfheaders: {
                    return bitcoin_p2p_builder<cfheaders::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::cfilter: {
                    return bitcoin_p2p_builder<cfilter::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getaddr: {
                    return bitcoin_p2p_builder<getaddr::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getblocks: {
                    return bitcoin_p2p_builder<getblocks::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getcfcheckpt: {
                    return bitcoin_p2p_builder<getcfcheckpt::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getcfheaders: {
                    return bitcoin_p2p_builder<getcfheaders::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getcfilters: {
                    return bitcoin_p2p_builder<getcfilters::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getdata: {
                    return bitcoin_p2p_builder<getdata::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::getheaders: {
                    return bitcoin_p2p_builder<getheaders::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::headers: {
                    return bitcoin_p2p_builder<headers::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::inv: {
                    return bitcoin_p2p_builder<inv::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::mempool: {
                    return bitcoin_p2p_builder<mempool::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::notfound: {
                    return bitcoin_p2p_builder<notfound::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::ping: {
                    return bitcoin_p2p_builder<ping::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::pong: {
                    return bitcoin_p2p_builder<pong::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::reject: {
                    return bitcoin_p2p_builder<reject::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::sendaddr2: {
                    return bitcoin_p2p_builder<sendaddr2::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::tx: {
                    return bitcoin_p2p_builder<tx::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::verack: {
                    return bitcoin_p2p_builder<verack::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::version: {
                    return bitcoin_p2p_builder<version::Message>(
                        api,
                        chain,
                        command,
                        header.Checksum(),
                        payloadBytes,
                        alloc);
                }
                case Command::unknown:
                case Command::alert:
                case Command::blocktxn:
                case Command::checkorder:
                case Command::cmpctblock:
                case Command::feefilter:
                case Command::filteradd:
                case Command::filterclear:
                case Command::filterload:
                case Command::getblocktxn:
                case Command::merkleblock:
                case Command::reply:
                case Command::sendcmpct:
                case Command::sendheaders:
                case Command::submitorder:
                default: {

                    return blank();
                }
            }
        } catch (const std::exception& e) {
            LogError()("opentxs::factory::")(__func__)(": ")(e.what())(
                " while processing ")(print(header.Command()))
                .Flush();

            return blank();
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {alloc};
    }
}
}  // namespace opentxs::factory
