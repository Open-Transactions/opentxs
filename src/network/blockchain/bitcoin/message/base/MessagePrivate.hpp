// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
namespace message
{
namespace addr2
{
class MessagePrivate;
}  // namespace addr2

namespace addr
{
class MessagePrivate;
}  // namespace addr

namespace block
{
class MessagePrivate;
}  // namespace block

namespace cfcheckpt
{
class MessagePrivate;
}  // namespace cfcheckpt

namespace cfheaders
{
class MessagePrivate;
}  // namespace cfheaders

namespace cfilter
{
class MessagePrivate;
}  // namespace cfilter

namespace getaddr
{
class MessagePrivate;
}  // namespace getaddr

namespace getblocks
{
class MessagePrivate;
}  // namespace getblocks

namespace getcfcheckpt
{
class MessagePrivate;
}  // namespace getcfcheckpt

namespace getcfheaders
{
class MessagePrivate;
}  // namespace getcfheaders

namespace getcfilters
{
class MessagePrivate;
}  // namespace getcfilters

namespace getdata
{
class MessagePrivate;
}  // namespace getdata

namespace getheaders
{
class MessagePrivate;
}  // namespace getheaders

namespace headers
{
class MessagePrivate;
}  // namespace headers

namespace internal
{
class Addr2;
class Addr;
class Block;
class Cfcheckpt;
class Cfheaders;
class Cfilter;
class Getaddr;
class Getblocks;
class Getcfcheckpt;
class Getcfheaders;
class Getcfilters;
class Getdata;
class Getheaders;
class Headers;
class Inv;
class Mempool;
class Message;
class Notfound;
class Ping;
class Pong;
class Reject;
class Sendaddr2;
class Tx;
class Verack;
class Version;
}  // namespace internal

namespace inv
{
class MessagePrivate;
}  // namespace inv

namespace mempool
{
class MessagePrivate;
}  // namespace mempool

namespace notfound
{
class MessagePrivate;
}  // namespace notfound

namespace ping
{
class MessagePrivate;
}  // namespace ping

namespace pong
{
class MessagePrivate;
}  // namespace pong

namespace reject
{
class MessagePrivate;
}  // namespace reject

namespace sendaddr2
{
class MessagePrivate;
}  // namespace sendaddr2

namespace tx
{
class MessagePrivate;
}  // namespace tx

namespace verack
{
class MessagePrivate;
}  // namespace verack

namespace version
{
class MessagePrivate;
}  // namespace version
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class MessagePrivate : public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> MessagePrivate*
    {
        return pmr::default_construct<MessagePrivate>({alloc});
    }
    static auto Reset(internal::Message& message) noexcept -> void;

    virtual auto asAddr2Private() const noexcept
        -> const addr2::MessagePrivate*;
    virtual auto asAddr2Public() const noexcept -> const Addr2&;
    virtual auto asAddrPrivate() const noexcept -> const addr::MessagePrivate*;
    virtual auto asAddrPublic() const noexcept -> const Addr&;
    virtual auto asBlockPrivate() const noexcept
        -> const block::MessagePrivate*;
    virtual auto asBlockPublic() const noexcept -> const Block&;
    virtual auto asCfcheckptPrivate() const noexcept
        -> const cfcheckpt::MessagePrivate*;
    virtual auto asCfcheckptPublic() const noexcept -> const Cfcheckpt&;
    virtual auto asCfheadersPrivate() const noexcept
        -> const cfheaders::MessagePrivate*;
    virtual auto asCfheadersPublic() const noexcept -> const Cfheaders&;
    virtual auto asCfilterPrivate() const noexcept
        -> const cfilter::MessagePrivate*;
    virtual auto asCfilterPublic() const noexcept -> const Cfilter&;
    virtual auto asGetaddrPrivate() const noexcept
        -> const getaddr::MessagePrivate*;
    virtual auto asGetaddrPublic() const noexcept -> const Getaddr&;
    virtual auto asGetblocksPrivate() const noexcept
        -> const getblocks::MessagePrivate*;
    virtual auto asGetblocksPublic() const noexcept -> const Getblocks&;
    virtual auto asGetcfcheckptPrivate() const noexcept
        -> const getcfcheckpt::MessagePrivate*;
    virtual auto asGetcfcheckptPublic() const noexcept -> const Getcfcheckpt&;
    virtual auto asGetcfheadersPrivate() const noexcept
        -> const getcfheaders::MessagePrivate*;
    virtual auto asGetcfheadersPublic() const noexcept -> const Getcfheaders&;
    virtual auto asGetcfiltersPrivate() const noexcept
        -> const getcfilters::MessagePrivate*;
    virtual auto asGetcfiltersPublic() const noexcept -> const Getcfilters&;
    virtual auto asGetdataPrivate() const noexcept
        -> const getdata::MessagePrivate*;
    virtual auto asGetdataPublic() const noexcept -> const Getdata&;
    virtual auto asGetheadersPrivate() const noexcept
        -> const getheaders::MessagePrivate*;
    virtual auto asGetheadersPublic() const noexcept -> const Getheaders&;
    virtual auto asHeadersPrivate() const noexcept
        -> const headers::MessagePrivate*;
    virtual auto asHeadersPublic() const noexcept -> const Headers&;
    virtual auto asInvPrivate() const noexcept -> const inv::MessagePrivate*;
    virtual auto asInvPublic() const noexcept -> const Inv&;
    virtual auto asMempoolPrivate() const noexcept
        -> const mempool::MessagePrivate*;
    virtual auto asMempoolPublic() const noexcept -> const Mempool&;
    virtual auto asNotfoundPrivate() const noexcept
        -> const notfound::MessagePrivate*;
    virtual auto asNotfoundPublic() const noexcept -> const Notfound&;
    virtual auto asPingPrivate() const noexcept -> const ping::MessagePrivate*;
    virtual auto asPingPublic() const noexcept -> const Ping&;
    virtual auto asPongPrivate() const noexcept -> const pong::MessagePrivate*;
    virtual auto asPongPublic() const noexcept -> const Pong&;
    virtual auto asRejectPrivate() const noexcept
        -> const reject::MessagePrivate*;
    virtual auto asRejectPublic() const noexcept -> const Reject&;
    virtual auto asSendaddr2Private() const noexcept
        -> const sendaddr2::MessagePrivate*;
    virtual auto asSendaddr2Public() const noexcept -> const Sendaddr2&;
    virtual auto asTxPrivate() const noexcept -> const tx::MessagePrivate*;
    virtual auto asTxPublic() const noexcept -> const Tx&;
    virtual auto asVerackPrivate() const noexcept
        -> const verack::MessagePrivate*;
    virtual auto asVerackPublic() const noexcept -> const Verack&;
    virtual auto asVersionPrivate() const noexcept
        -> const version::MessagePrivate*;
    virtual auto asVersionPublic() const noexcept -> const Version&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> MessagePrivate*
    {
        return pmr::clone(this, {alloc});
    }
    virtual auto Command() const noexcept -> message::Command;
    virtual auto Describe() const noexcept -> ReadView;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Network() const noexcept -> opentxs::blockchain::Type;
    virtual auto Transmit(Transport type, zeromq::Message& out) const
        noexcept(false) -> void;

    virtual auto asAddr2Public() noexcept -> Addr2&;
    virtual auto asAddrPublic() noexcept -> Addr&;
    virtual auto asBlockPublic() noexcept -> Block&;
    virtual auto asCfcheckptPublic() noexcept -> Cfcheckpt&;
    virtual auto asCfheadersPublic() noexcept -> Cfheaders&;
    virtual auto asCfilterPublic() noexcept -> Cfilter&;
    virtual auto asGetaddrPublic() noexcept -> Getaddr&;
    virtual auto asGetblocksPublic() noexcept -> Getblocks&;
    virtual auto asGetcfcheckptPublic() noexcept -> Getcfcheckpt&;
    virtual auto asGetcfheadersPublic() noexcept -> Getcfheaders&;
    virtual auto asGetcfiltersPublic() noexcept -> Getcfilters&;
    virtual auto asGetdataPublic() noexcept -> Getdata&;
    virtual auto asGetheadersPublic() noexcept -> Getheaders&;
    virtual auto asHeadersPublic() noexcept -> Headers&;
    virtual auto asInvPublic() noexcept -> Inv&;
    virtual auto asMempoolPublic() noexcept -> Mempool&;
    virtual auto asNotfoundPublic() noexcept -> Notfound&;
    virtual auto asPingPublic() noexcept -> Ping&;
    virtual auto asPongPublic() noexcept -> Pong&;
    virtual auto asRejectPublic() noexcept -> Reject&;
    virtual auto asSendaddr2Public() noexcept -> Sendaddr2&;
    virtual auto asTxPublic() noexcept -> Tx&;
    virtual auto asVerackPublic() noexcept -> Verack&;
    virtual auto asVersionPublic() noexcept -> Version&;
    virtual auto asAddr2Private() noexcept -> addr2::MessagePrivate*;
    virtual auto asAddrPrivate() noexcept -> addr::MessagePrivate*;
    virtual auto asBlockPrivate() noexcept -> block::MessagePrivate*;
    virtual auto asCfcheckptPrivate() noexcept -> cfcheckpt::MessagePrivate*;
    virtual auto asCfheadersPrivate() noexcept -> cfheaders::MessagePrivate*;
    virtual auto asCfilterPrivate() noexcept -> cfilter::MessagePrivate*;
    virtual auto asGetaddrPrivate() noexcept -> getaddr::MessagePrivate*;
    virtual auto asGetblocksPrivate() noexcept -> getblocks::MessagePrivate*;
    virtual auto asGetcfcheckptPrivate() noexcept
        -> getcfcheckpt::MessagePrivate*;
    virtual auto asGetcfheadersPrivate() noexcept
        -> getcfheaders::MessagePrivate*;
    virtual auto asGetcfiltersPrivate() noexcept
        -> getcfilters::MessagePrivate*;
    virtual auto asGetdataPrivate() noexcept -> getdata::MessagePrivate*;
    virtual auto asGetheadersPrivate() noexcept -> getheaders::MessagePrivate*;
    virtual auto asHeadersPrivate() noexcept -> headers::MessagePrivate*;
    virtual auto asInvPrivate() noexcept -> inv::MessagePrivate*;
    virtual auto asMempoolPrivate() noexcept -> mempool::MessagePrivate*;
    virtual auto asNotfoundPrivate() noexcept -> notfound::MessagePrivate*;
    virtual auto asPingPrivate() noexcept -> ping::MessagePrivate*;
    virtual auto asPongPrivate() noexcept -> pong::MessagePrivate*;
    virtual auto asRejectPrivate() noexcept -> reject::MessagePrivate*;
    virtual auto asSendaddr2Private() noexcept -> sendaddr2::MessagePrivate*;
    virtual auto asTxPrivate() noexcept -> tx::MessagePrivate*;
    virtual auto asVerackPrivate() noexcept -> verack::MessagePrivate*;
    virtual auto asVersionPrivate() noexcept -> version::MessagePrivate*;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    MessagePrivate(allocator_type alloc) noexcept;
    MessagePrivate() = delete;
    MessagePrivate(const MessagePrivate& rhs, allocator_type alloc) noexcept;
    MessagePrivate(const MessagePrivate&) = delete;
    MessagePrivate(MessagePrivate&&) = delete;
    auto operator=(const MessagePrivate&) -> MessagePrivate& = delete;
    auto operator=(MessagePrivate&&) -> MessagePrivate& = delete;

    ~MessagePrivate() override;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
