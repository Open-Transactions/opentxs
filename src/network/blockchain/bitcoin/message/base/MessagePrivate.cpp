// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

#include <string_view>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Addr.hpp"
#include "internal/network/blockchain/bitcoin/message/Addr2.hpp"
#include "internal/network/blockchain/bitcoin/message/Block.hpp"
#include "internal/network/blockchain/bitcoin/message/Cfcheckpt.hpp"
#include "internal/network/blockchain/bitcoin/message/Cfheaders.hpp"
#include "internal/network/blockchain/bitcoin/message/Cfilter.hpp"
#include "internal/network/blockchain/bitcoin/message/Getaddr.hpp"
#include "internal/network/blockchain/bitcoin/message/Getblocks.hpp"
#include "internal/network/blockchain/bitcoin/message/Getcfcheckpt.hpp"
#include "internal/network/blockchain/bitcoin/message/Getcfheaders.hpp"
#include "internal/network/blockchain/bitcoin/message/Getcfilters.hpp"
#include "internal/network/blockchain/bitcoin/message/Getdata.hpp"
#include "internal/network/blockchain/bitcoin/message/Getheaders.hpp"
#include "internal/network/blockchain/bitcoin/message/Headers.hpp"
#include "internal/network/blockchain/bitcoin/message/Inv.hpp"
#include "internal/network/blockchain/bitcoin/message/Mempool.hpp"
#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Notfound.hpp"
#include "internal/network/blockchain/bitcoin/message/Ping.hpp"
#include "internal/network/blockchain/bitcoin/message/Pong.hpp"
#include "internal/network/blockchain/bitcoin/message/Reject.hpp"
#include "internal/network/blockchain/bitcoin/message/Sendaddr2.hpp"
#include "internal/network/blockchain/bitcoin/message/Tx.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Verack.hpp"
#include "internal/network/blockchain/bitcoin/message/Version.hpp"
#include "network/blockchain/bitcoin/message/addr/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/addr2/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/block/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfcheckpt/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfheaders/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/cfilter/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getaddr/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getblocks/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfcheckpt/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfheaders/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getcfilters/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getdata/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getheaders/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/headers/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/inv/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/mempool/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/notfound/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/ping/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/pong/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/reject/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/sendaddr2/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/tx/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/verack/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/version/MessagePrivate.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain::bitcoin::message::internal
{
using namespace std::literals;

MessagePrivate::MessagePrivate(allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
{
}

MessagePrivate::MessagePrivate(
    const MessagePrivate&,
    allocator_type alloc) noexcept
    : MessagePrivate(std::move(alloc))
{
}

auto MessagePrivate::asAddr2Private() const noexcept
    -> const addr2::MessagePrivate*
{
    return addr2::MessagePrivate::Blank({});
}

auto MessagePrivate::asAddr2Private() noexcept -> addr2::MessagePrivate*
{
    return addr2::MessagePrivate::Blank({});
}

auto MessagePrivate::asAddr2Public() const noexcept -> const Addr2&
{
    return Addr2::Blank();
}

auto MessagePrivate::asAddr2Public() noexcept -> Addr2&
{
    return Addr2::Blank();
}

auto MessagePrivate::asAddrPrivate() const noexcept
    -> const addr::MessagePrivate*
{
    return addr::MessagePrivate::Blank({});
}

auto MessagePrivate::asAddrPrivate() noexcept -> addr::MessagePrivate*
{
    return addr::MessagePrivate::Blank({});
}

auto MessagePrivate::asAddrPublic() const noexcept -> const Addr&
{
    return Addr::Blank();
}

auto MessagePrivate::asAddrPublic() noexcept -> Addr& { return Addr::Blank(); }

auto MessagePrivate::asBlockPrivate() const noexcept
    -> const block::MessagePrivate*
{
    return block::MessagePrivate::Blank({});
}

auto MessagePrivate::asBlockPrivate() noexcept -> block::MessagePrivate*
{
    return block::MessagePrivate::Blank({});
}

auto MessagePrivate::asBlockPublic() const noexcept -> const Block&
{
    return Block::Blank();
}

auto MessagePrivate::asBlockPublic() noexcept -> Block&
{
    return Block::Blank();
}

auto MessagePrivate::asCfcheckptPrivate() const noexcept
    -> const cfcheckpt::MessagePrivate*
{
    return cfcheckpt::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfcheckptPrivate() noexcept -> cfcheckpt::MessagePrivate*
{
    return cfcheckpt::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfcheckptPublic() const noexcept -> const Cfcheckpt&
{
    return Cfcheckpt::Blank();
}

auto MessagePrivate::asCfcheckptPublic() noexcept -> Cfcheckpt&
{
    return Cfcheckpt::Blank();
}

auto MessagePrivate::asCfheadersPrivate() const noexcept
    -> const cfheaders::MessagePrivate*
{
    return cfheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfheadersPrivate() noexcept -> cfheaders::MessagePrivate*
{
    return cfheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfheadersPublic() const noexcept -> const Cfheaders&
{
    return Cfheaders::Blank();
}

auto MessagePrivate::asCfheadersPublic() noexcept -> Cfheaders&
{
    return Cfheaders::Blank();
}

auto MessagePrivate::asCfilterPrivate() const noexcept
    -> const cfilter::MessagePrivate*
{
    return cfilter::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfilterPrivate() noexcept -> cfilter::MessagePrivate*
{
    return cfilter::MessagePrivate::Blank({});
}

auto MessagePrivate::asCfilterPublic() const noexcept -> const Cfilter&
{
    return Cfilter::Blank();
}

auto MessagePrivate::asCfilterPublic() noexcept -> Cfilter&
{
    return Cfilter::Blank();
}

auto MessagePrivate::asGetaddrPrivate() const noexcept
    -> const getaddr::MessagePrivate*
{
    return getaddr::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetaddrPrivate() noexcept -> getaddr::MessagePrivate*
{
    return getaddr::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetaddrPublic() const noexcept -> const Getaddr&
{
    return Getaddr::Blank();
}

auto MessagePrivate::asGetaddrPublic() noexcept -> Getaddr&
{
    return Getaddr::Blank();
}

auto MessagePrivate::asGetblocksPrivate() const noexcept
    -> const getblocks::MessagePrivate*
{
    return getblocks::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetblocksPrivate() noexcept -> getblocks::MessagePrivate*
{
    return getblocks::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetblocksPublic() const noexcept -> const Getblocks&
{
    return Getblocks::Blank();
}

auto MessagePrivate::asGetblocksPublic() noexcept -> Getblocks&
{
    return Getblocks::Blank();
}

auto MessagePrivate::asGetcfcheckptPrivate() const noexcept
    -> const getcfcheckpt::MessagePrivate*
{
    return getcfcheckpt::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfcheckptPrivate() noexcept
    -> getcfcheckpt::MessagePrivate*
{
    return getcfcheckpt::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfcheckptPublic() const noexcept
    -> const Getcfcheckpt&
{
    return Getcfcheckpt::Blank();
}

auto MessagePrivate::asGetcfcheckptPublic() noexcept -> Getcfcheckpt&
{
    return Getcfcheckpt::Blank();
}

auto MessagePrivate::asGetcfheadersPrivate() const noexcept
    -> const getcfheaders::MessagePrivate*
{
    return getcfheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfheadersPrivate() noexcept
    -> getcfheaders::MessagePrivate*
{
    return getcfheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfheadersPublic() const noexcept
    -> const Getcfheaders&
{
    return Getcfheaders::Blank();
}

auto MessagePrivate::asGetcfheadersPublic() noexcept -> Getcfheaders&
{
    return Getcfheaders::Blank();
}

auto MessagePrivate::asGetcfiltersPrivate() const noexcept
    -> const getcfilters::MessagePrivate*
{
    return getcfilters::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfiltersPrivate() noexcept
    -> getcfilters::MessagePrivate*
{
    return getcfilters::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetcfiltersPublic() const noexcept -> const Getcfilters&
{
    return Getcfilters::Blank();
}

auto MessagePrivate::asGetcfiltersPublic() noexcept -> Getcfilters&
{
    return Getcfilters::Blank();
}

auto MessagePrivate::asGetdataPrivate() const noexcept
    -> const getdata::MessagePrivate*
{
    return getdata::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetdataPrivate() noexcept -> getdata::MessagePrivate*
{
    return getdata::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetdataPublic() const noexcept -> const Getdata&
{
    return Getdata::Blank();
}

auto MessagePrivate::asGetdataPublic() noexcept -> Getdata&
{
    return Getdata::Blank();
}

auto MessagePrivate::asGetheadersPrivate() const noexcept
    -> const getheaders::MessagePrivate*
{
    return getheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetheadersPrivate() noexcept
    -> getheaders::MessagePrivate*
{
    return getheaders::MessagePrivate::Blank({});
}

auto MessagePrivate::asGetheadersPublic() const noexcept -> const Getheaders&
{
    return Getheaders::Blank();
}

auto MessagePrivate::asGetheadersPublic() noexcept -> Getheaders&
{
    return Getheaders::Blank();
}

auto MessagePrivate::asHeadersPrivate() const noexcept
    -> const headers::MessagePrivate*
{
    return headers::MessagePrivate::Blank({});
}

auto MessagePrivate::asHeadersPrivate() noexcept -> headers::MessagePrivate*
{
    return headers::MessagePrivate::Blank({});
}

auto MessagePrivate::asHeadersPublic() const noexcept -> const Headers&
{
    return Headers::Blank();
}

auto MessagePrivate::asHeadersPublic() noexcept -> Headers&
{
    return Headers::Blank();
}

auto MessagePrivate::asInvPrivate() const noexcept -> const inv::MessagePrivate*
{
    return inv::MessagePrivate::Blank({});
}

auto MessagePrivate::asInvPrivate() noexcept -> inv::MessagePrivate*
{
    return inv::MessagePrivate::Blank({});
}

auto MessagePrivate::asInvPublic() const noexcept -> const Inv&
{
    return Inv::Blank();
}

auto MessagePrivate::asInvPublic() noexcept -> Inv& { return Inv::Blank(); }

auto MessagePrivate::asMempoolPrivate() const noexcept
    -> const mempool::MessagePrivate*
{
    return mempool::MessagePrivate::Blank({});
}

auto MessagePrivate::asMempoolPrivate() noexcept -> mempool::MessagePrivate*
{
    return mempool::MessagePrivate::Blank({});
}

auto MessagePrivate::asMempoolPublic() const noexcept -> const Mempool&
{
    return Mempool::Blank();
}

auto MessagePrivate::asMempoolPublic() noexcept -> Mempool&
{
    return Mempool::Blank();
}

auto MessagePrivate::asNotfoundPrivate() const noexcept
    -> const notfound::MessagePrivate*
{
    return notfound::MessagePrivate::Blank({});
}

auto MessagePrivate::asNotfoundPrivate() noexcept -> notfound::MessagePrivate*
{
    return notfound::MessagePrivate::Blank({});
}

auto MessagePrivate::asNotfoundPublic() const noexcept -> const Notfound&
{
    return Notfound::Blank();
}

auto MessagePrivate::asNotfoundPublic() noexcept -> Notfound&
{
    return Notfound::Blank();
}

auto MessagePrivate::asPingPrivate() const noexcept
    -> const ping::MessagePrivate*
{
    return ping::MessagePrivate::Blank({});
}

auto MessagePrivate::asPingPrivate() noexcept -> ping::MessagePrivate*
{
    return ping::MessagePrivate::Blank({});
}

auto MessagePrivate::asPingPublic() const noexcept -> const Ping&
{
    return Ping::Blank();
}

auto MessagePrivate::asPingPublic() noexcept -> Ping& { return Ping::Blank(); }

auto MessagePrivate::asPongPrivate() const noexcept
    -> const pong::MessagePrivate*
{
    return pong::MessagePrivate::Blank({});
}

auto MessagePrivate::asPongPrivate() noexcept -> pong::MessagePrivate*
{
    return pong::MessagePrivate::Blank({});
}

auto MessagePrivate::asPongPublic() const noexcept -> const Pong&
{
    return Pong::Blank();
}

auto MessagePrivate::asPongPublic() noexcept -> Pong& { return Pong::Blank(); }

auto MessagePrivate::asRejectPrivate() const noexcept
    -> const reject::MessagePrivate*
{
    return reject::MessagePrivate::Blank({});
}

auto MessagePrivate::asRejectPrivate() noexcept -> reject::MessagePrivate*
{
    return reject::MessagePrivate::Blank({});
}

auto MessagePrivate::asRejectPublic() const noexcept -> const Reject&
{
    return Reject::Blank();
}

auto MessagePrivate::asRejectPublic() noexcept -> Reject&
{
    return Reject::Blank();
}

auto MessagePrivate::asSendaddr2Private() const noexcept
    -> const sendaddr2::MessagePrivate*
{
    return sendaddr2::MessagePrivate::Blank({});
}

auto MessagePrivate::asSendaddr2Private() noexcept -> sendaddr2::MessagePrivate*
{
    return sendaddr2::MessagePrivate::Blank({});
}

auto MessagePrivate::asSendaddr2Public() const noexcept -> const Sendaddr2&
{
    return Sendaddr2::Blank();
}

auto MessagePrivate::asSendaddr2Public() noexcept -> Sendaddr2&
{
    return Sendaddr2::Blank();
}

auto MessagePrivate::asTxPrivate() const noexcept -> const tx::MessagePrivate*
{
    return tx::MessagePrivate::Blank({});
}

auto MessagePrivate::asTxPrivate() noexcept -> tx::MessagePrivate*
{
    return tx::MessagePrivate::Blank({});
}

auto MessagePrivate::asTxPublic() const noexcept -> const Tx&
{
    return Tx::Blank();
}

auto MessagePrivate::asTxPublic() noexcept -> Tx& { return Tx::Blank(); }

auto MessagePrivate::asVerackPrivate() const noexcept
    -> const verack::MessagePrivate*
{
    return verack::MessagePrivate::Blank({});
}

auto MessagePrivate::asVerackPrivate() noexcept -> verack::MessagePrivate*
{
    return verack::MessagePrivate::Blank({});
}

auto MessagePrivate::asVerackPublic() const noexcept -> const Verack&
{
    return Verack::Blank();
}

auto MessagePrivate::asVerackPublic() noexcept -> Verack&
{
    return Verack::Blank();
}

auto MessagePrivate::asVersionPrivate() const noexcept
    -> const version::MessagePrivate*
{
    return version::MessagePrivate::Blank({});
}

auto MessagePrivate::asVersionPrivate() noexcept -> version::MessagePrivate*
{
    return version::MessagePrivate::Blank({});
}

auto MessagePrivate::asVersionPublic() const noexcept -> const Version&
{
    return Version::Blank();
}

auto MessagePrivate::asVersionPublic() noexcept -> Version&
{
    return Version::Blank();
}

auto MessagePrivate::Command() const noexcept -> message::Command
{
    using enum message::Command;

    return unknown;
}

auto MessagePrivate::Describe() const noexcept -> ReadView
{
    return "unknown command"sv;
}

auto MessagePrivate::IsValid() const noexcept -> bool { return false; }

auto MessagePrivate::Network() const noexcept -> opentxs::blockchain::Type
{
    using enum opentxs::blockchain::Type;

    return UnknownBlockchain;
}

auto MessagePrivate::Transmit(Transport type, zeromq::Message& out) const
    noexcept(false) -> void
{
}

auto MessagePrivate::Reset(internal::Message& message) noexcept -> void
{
    message.imp_ = nullptr;
}

MessagePrivate::~MessagePrivate() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
