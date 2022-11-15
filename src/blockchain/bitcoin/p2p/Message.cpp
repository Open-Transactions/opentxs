// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/Message.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/message/Cmpctblock.hpp"
#include "blockchain/bitcoin/p2p/message/Feefilter.hpp"
#include "blockchain/bitcoin/p2p/message/Getblocks.hpp"
#include "blockchain/bitcoin/p2p/message/Getblocktxn.hpp"
#include "blockchain/bitcoin/p2p/message/Merkleblock.hpp"
#include "blockchain/bitcoin/p2p/message/Reject.hpp"
#include "blockchain/bitcoin/p2p/message/Sendcmpct.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PMessage(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::Message*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::Message;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    ReturnType* pMessage{nullptr};

    switch (pHeader->Command()) {
        case bitcoin::Command::addr: {
            pMessage = BitcoinP2PAddr(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::block: {
            pMessage = BitcoinP2PBlock(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::blocktxn: {
            pMessage = BitcoinP2PBlocktxn(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::cmpctblock: {
            pMessage = BitcoinP2PCmpctblock(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::feefilter: {
            pMessage = BitcoinP2PFeefilter(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::filteradd: {
            pMessage = BitcoinP2PFilteradd(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::filterclear: {
            pMessage = BitcoinP2PFilterclear(api, std::move(pHeader));
        } break;
        case bitcoin::Command::filterload: {
            pMessage = BitcoinP2PFilterload(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getaddr: {
            pMessage = BitcoinP2PGetaddr(api, std::move(pHeader));
        } break;
        case bitcoin::Command::getblocks: {
            pMessage = BitcoinP2PGetblocks(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getblocktxn: {
            pMessage = BitcoinP2PGetblocktxn(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getdata: {
            pMessage = BitcoinP2PGetdata(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getheaders: {
            pMessage = BitcoinP2PGetheaders(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::headers: {
            pMessage = BitcoinP2PHeaders(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::inv: {
            pMessage = BitcoinP2PInv(
                           api,
                           std::move(pHeader),
                           version,
                           {static_cast<const char*>(payload), size})
                           .release();
        } break;
        case bitcoin::Command::mempool: {
            pMessage = BitcoinP2PMempool(api, std::move(pHeader));
        } break;
        case bitcoin::Command::merkleblock: {
            pMessage = BitcoinP2PMerkleblock(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::notfound: {
            pMessage = BitcoinP2PNotfound(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::ping: {
            pMessage = BitcoinP2PPing(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::pong: {
            pMessage = BitcoinP2PPong(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::reject: {
            pMessage = BitcoinP2PReject(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::sendcmpct: {
            pMessage = BitcoinP2PSendcmpct(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::sendheaders: {
            pMessage = BitcoinP2PSendheaders(api, std::move(pHeader));
        } break;
        case bitcoin::Command::tx: {
            pMessage = BitcoinP2PTx(
                           api,
                           std::move(pHeader),
                           version,
                           {static_cast<const char*>(payload), size})
                           .release();
        } break;
        case bitcoin::Command::verack: {
            pMessage = BitcoinP2PVerack(api, std::move(pHeader));
        } break;
        case bitcoin::Command::version: {
            pMessage = BitcoinP2PVersion(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getcfilters: {
            pMessage = BitcoinP2PGetcfilters(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::cfilter: {
            pMessage = BitcoinP2PCfilter(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getcfheaders: {
            pMessage = BitcoinP2PGetcfheaders(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::cfheaders: {
            pMessage = BitcoinP2PCfheaders(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::getcfcheckpt: {
            pMessage = BitcoinP2PGetcfcheckpt(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::cfcheckpt: {
            pMessage = BitcoinP2PCfcheckpt(
                api,
                std::move(pHeader),
                version,
                {static_cast<const char*>(payload), size});
        } break;
        case bitcoin::Command::alert:
        case bitcoin::Command::checkorder:
        case bitcoin::Command::reply:
        case bitcoin::Command::submitorder:
        case bitcoin::Command::unknown:
        default: {
            LogError()("opentxs::factory::")(__func__)(
                ": Unsupported message type")
                .Flush();
            return nullptr;
        }
    }

    return pMessage;
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin
{
auto Message::MaxPayload() -> std::size_t
{
    static_assert(
        std::numeric_limits<std::size_t>::max() >=
        std::numeric_limits<std::uint32_t>::max());

    return std::numeric_limits<std::uint32_t>::max();
}
}  // namespace opentxs::blockchain::p2p::bitcoin

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Message::Message(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::Command command) noexcept
    : api_(api)
    , header_(std::make_unique<Header>(api, network, command))
{
    OT_ASSERT(header_);
}

Message::Message(
    const api::Session& api,
    std::unique_ptr<Header> header) noexcept
    : api_(api)
    , header_(std::move(header))
{
    OT_ASSERT(header_);
}

auto Message::Encode() const -> ByteArray
{
    auto output = api_.Factory().Data();
    header().Serialize(output.WriteInto());
    output += payload();

    return output;
}

auto Message::calculate_checksum(const Data& payload) const noexcept
    -> ByteArray
{
    auto output = ByteArray{};
    P2PMessageHash(
        api_.Crypto(), header().Network(), payload.Bytes(), output.WriteInto());

    return output;
}

auto Message::init_hash() noexcept -> void
{
    const auto data = payload();
    const auto size = data.size();
    header().SetChecksum(size, calculate_checksum(data));
}

auto Message::payload() const noexcept -> ByteArray
{
    auto out = api_.Factory().Data();
    payload(out.WriteInto());

    return out;
}

auto Message::Transmit() const noexcept -> std::pair<zmq::Frame, zmq::Frame>
{
    auto output = std::pair<zmq::Frame, zmq::Frame>{};
    auto& [header, payload] = output;
    this->header().Serialize(header.WriteInto());
    this->payload(payload.WriteInto());

    return output;
}

auto Message::verify_checksum() const noexcept(false) -> void
{
    const auto calculated = calculate_checksum(payload());
    const auto& provided = header().Checksum();

    if (provided != calculated) {
        LogError()(OT_PRETTY_CLASS())("Checksum failure").Flush();
        LogError()("*  Calculated Payload:  ").asHex(payload()).Flush();
        LogError()("*  Calculated Checksum: ").asHex(calculated).Flush();
        LogError()("*  Provided Checksum:   ").asHex(provided).Flush();

        throw std::runtime_error("checksum failure");
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
