// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/blockchain/bitcoin/Peer.hpp"  // IWYU pragma: associated

#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"

namespace opentxs::network::blockchain::bitcoin
{
using enum opentxs::blockchain::p2p::bitcoin::Command;
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Addr> {
    static auto Name() noexcept { return print(addr); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Addr*
    {
        return factory::BitcoinP2PAddr(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Addr2> {
    static auto Name() noexcept { return print(Command::addr2); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Addr2*
    {
        return factory::BitcoinP2PAddr2Temp(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Block> {
    static auto Name() noexcept { return print(block); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Block*
    {
        return factory::BitcoinP2PBlock(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Blocktxn> {
    static auto Name() noexcept { return print(blocktxn); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Blocktxn*
    {
        return factory::BitcoinP2PBlocktxn(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Cfcheckpt> {
    static auto Name() noexcept { return print(cfcheckpt); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Cfcheckpt*
    {
        return factory::BitcoinP2PCfcheckpt(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Cfheaders> {
    static auto Name() noexcept { return print(cfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Cfheaders*
    {
        return factory::BitcoinP2PCfheaders(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Cfilter> {
    static auto Name() noexcept { return print(cfilter); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Cfilter*
    {
        return factory::BitcoinP2PCfilter(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Cmpctblock> {
    static auto Name() noexcept { return print(cmpctblock); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Cmpctblock*
    {
        return factory::BitcoinP2PCmpctblock(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Feefilter> {
    static auto Name() noexcept { return print(feefilter); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Feefilter*
    {
        return factory::BitcoinP2PFeefilter(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Filteradd> {
    static auto Name() noexcept { return print(filteradd); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Filteradd*
    {
        return factory::BitcoinP2PFilteradd(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Filterclear> {
    static auto Name() noexcept { return print(filterclear); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Filterclear*
    {
        return factory::BitcoinP2PFilterclear(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Filterload> {
    static auto Name() noexcept { return print(filterload); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Filterload*
    {
        return factory::BitcoinP2PFilterload(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getaddr> {
    static auto Name() noexcept { return print(getaddr); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getaddr*
    {
        return factory::BitcoinP2PGetaddr(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Getblocks> {
    static auto Name() noexcept { return print(getblocks); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Getblocks*
    {
        return factory::BitcoinP2PGetblocks(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Getblocktxn> {
    static auto Name() noexcept { return print(getblocktxn); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Getblocktxn*
    {
        return factory::BitcoinP2PGetblocktxn(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getcfcheckpt> {
    static auto Name() noexcept { return print(getcfcheckpt); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*
    {
        return factory::BitcoinP2PGetcfcheckpt(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getcfheaders> {
    static auto Name() noexcept { return print(getcfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getcfheaders*
    {
        return factory::BitcoinP2PGetcfheaders(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getcfilters> {
    static auto Name() noexcept { return print(getcfilters); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getcfilters*
    {
        return factory::BitcoinP2PGetcfilters(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getdata> {
    static auto Name() noexcept { return print(getdata); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getdata*
    {
        return factory::BitcoinP2PGetdata(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getheaders> {
    static auto Name() noexcept { return print(getheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Getheaders*
    {
        return factory::BitcoinP2PGetheaders(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Headers> {
    static auto Name() noexcept { return print(headers); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Headers*
    {
        return factory::BitcoinP2PHeaders(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Inv> {
    static auto Name() noexcept { return print(inv); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Inv*
    {
        return factory::BitcoinP2PInvTemp(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Mempool> {
    static auto Name() noexcept { return print(mempool); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Mempool*
    {
        return factory::BitcoinP2PMempool(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Merkleblock> {
    static auto Name() noexcept { return print(merkleblock); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Merkleblock*
    {
        return factory::BitcoinP2PMerkleblock(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Notfound> {
    static auto Name() noexcept { return print(notfound); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Notfound*
    {
        return factory::BitcoinP2PNotfound(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Ping> {
    static auto Name() noexcept { return print(ping); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Ping*
    {
        return factory::BitcoinP2PPing(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Pong> {
    static auto Name() noexcept { return print(pong); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Pong*
    {
        return factory::BitcoinP2PPong(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Reject> {
    static auto Name() noexcept { return print(reject); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Reject*
    {
        return factory::BitcoinP2PReject(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Sendaddr2> {
    static auto Name() noexcept { return print(Command::sendcmpct); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Sendaddr2*
    {
        return factory::BitcoinP2PSendaddr2(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<opentxs::blockchain::p2p::bitcoin::message::Sendcmpct> {
    static auto Name() noexcept { return print(sendcmpct); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::Sendcmpct*
    {
        return factory::BitcoinP2PSendcmpct(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Sendheaders> {
    static auto Name() noexcept { return print(sendheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Sendheaders*
    {
        return factory::BitcoinP2PSendheaders(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Tx> {
    static auto Name() noexcept { return print(tx); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Tx*
    {
        return factory::BitcoinP2PTxTemp(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Verack> {
    static auto Name() noexcept { return print(verack); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Verack*
    {
        return factory::BitcoinP2PVerack(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
template <>
struct Peer::FromWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Version> {
    static auto Name() noexcept { return print(version); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
        Args&&... args) const
        -> opentxs::blockchain::p2p::bitcoin::message::internal::Version*
    {
        return factory::BitcoinP2PVersion(
            api, std::move(header), std::forward<Args>(args)...);
    }
};
}  // namespace opentxs::network::blockchain::bitcoin

namespace opentxs::network::blockchain::bitcoin
{
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Block> {
    static auto Name() noexcept { return print(block); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Block>{
            factory::BitcoinP2PBlock(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Cfheaders> {
    static auto Name() noexcept { return print(cfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Cfheaders>{
            factory::BitcoinP2PCfheaders(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Cfilter> {
    static auto Name() noexcept { return print(cfilter); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Cfilter>{
            factory::BitcoinP2PCfilter(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getaddr> {
    static auto Name() noexcept { return print(getaddr); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Getaddr>{
            factory::BitcoinP2PGetaddr(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getcfheaders> {
    static auto Name() noexcept { return print(getcfheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Getcfheaders>{
            factory::BitcoinP2PGetcfheaders(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getcfilters> {
    static auto Name() noexcept { return print(getcfilters); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Getcfilters>{
            factory::BitcoinP2PGetcfilters(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getdata> {
    static auto Name() noexcept { return print(getdata); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Getdata>{
            factory::BitcoinP2PGetdata(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Getheaders> {
    static auto Name() noexcept { return print(getheaders); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Getheaders>{
            factory::BitcoinP2PGetheaders(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Headers> {
    static auto Name() noexcept { return print(headers); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Headers>{
            factory::BitcoinP2PHeaders(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<opentxs::blockchain::p2p::bitcoin::message::internal::Inv> {
    static auto Name() noexcept { return print(inv); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Inv>{
            factory::BitcoinP2PInv(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Mempool> {
    static auto Name() noexcept { return print(mempool); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Mempool>{
            factory::BitcoinP2PMempool(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Notfound> {
    static auto Name() noexcept { return print(notfound); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Notfound>{
            factory::BitcoinP2PNotfound(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Ping> {
    static auto Name() noexcept { return print(ping); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Ping>{
            factory::BitcoinP2PPing(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Pong> {
    static auto Name() noexcept { return print(pong); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Pong>{
            factory::BitcoinP2PPong(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Sendaddr2> {
    static auto Name() noexcept { return print(Command::sendaddr2); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Sendaddr2>{
            factory::BitcoinP2PSendaddr2(
                api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<opentxs::blockchain::p2p::bitcoin::message::internal::Tx> {
    static auto Name() noexcept { return print(tx); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Tx>{
            factory::BitcoinP2PTx(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Verack> {
    static auto Name() noexcept { return print(verack); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Verack>{
            factory::BitcoinP2PVerack(api, chain, std::forward<Args>(args)...)};
    }
};
template <>
struct Peer::ToWire<
    opentxs::blockchain::p2p::bitcoin::message::internal::Version> {
    static auto Name() noexcept { return print(version); }

    template <typename... Args>
    auto operator()(
        const api::Session& api,
        opentxs::blockchain::Type chain,
        Args&&... args) const
    {
        return std::unique_ptr<
            opentxs::blockchain::p2p::bitcoin::message::internal::Version>{
            factory::BitcoinP2PVersion(
                api, chain, std::forward<Args>(args)...)};
    }
};
}  // namespace opentxs::network::blockchain::bitcoin

namespace opentxs::network::blockchain::bitcoin
{
template <typename Incoming, typename... Args>
auto Peer::instantiate(std::unique_ptr<HeaderType> header, Args&&... args) const
    noexcept(false) -> std::unique_ptr<Incoming>
{
    OT_ASSERT(header);

    static const auto factory = FromWire<Incoming>{};
    auto out = std::unique_ptr<Incoming>{factory.operator()(
        api_, std::move(header), std::forward<Args>(args)...)};

    if (false == out.operator bool()) {
        auto error = CString{get_allocator()};
        error.append("failed to decode ");
        error.append(factory.Name());

        throw std::runtime_error{error.c_str()};
    }

    return out;
}

template <typename Outgoing, typename... Args>
auto Peer::transmit_protocol(Args&&... args) noexcept -> void
{
    static const auto factory = ToWire<Outgoing>{};

    try {
        const auto pMessage =
            factory(api_, chain_, std::forward<Args>(args)...);

        if (false == pMessage.operator bool()) {
            auto error = CString{get_allocator()};
            error.append("failed to construct ");
            error.append(factory.Name());

            throw std::runtime_error{error.c_str()};
        }
        const auto& message = *pMessage;
        log_(OT_PRETTY_CLASS())(name_)(": sending ")(factory.Name()).Flush();
        transmit(message.Transmit());
    } catch (const std::exception& e) {
        disconnect(e.what());

        return;
    }
}
}  // namespace opentxs::network::blockchain::bitcoin
