// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
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
class MessagePrivate;
class Notfound;
class Ping;
class Pong;
class Reject;
class Sendaddr2;
class Tx;
class Verack;
class Version;
}  // namespace internal
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class ByteArray;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Message : virtual public opentxs::Allocated
{
public:
    static auto Blank() noexcept -> Message&;
    static auto MaxPayload() -> std::size_t;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    auto asAddr2() const& noexcept -> const Addr2&;
    auto asAddr() const& noexcept -> const Addr&;
    auto asBlock() const& noexcept -> const Block&;
    auto asCfcheckpt() const& noexcept -> const Cfcheckpt&;
    auto asCfheaders() const& noexcept -> const Cfheaders&;
    auto asCfilter() const& noexcept -> const Cfilter&;
    auto asGetaddr() const& noexcept -> const Getaddr&;
    auto asGetblocks() const& noexcept -> const Getblocks&;
    auto asGetcfcheckpt() const& noexcept -> const Getcfcheckpt&;
    auto asGetcfheaders() const& noexcept -> const Getcfheaders&;
    auto asGetcfilters() const& noexcept -> const Getcfilters&;
    auto asGetdata() const& noexcept -> const Getdata&;
    auto asGetheaders() const& noexcept -> const Getheaders&;
    auto asHeaders() const& noexcept -> const Headers&;
    auto asInv() const& noexcept -> const Inv&;
    auto asMempool() const& noexcept -> const Mempool&;
    auto asNotfound() const& noexcept -> const Notfound&;
    auto asPing() const& noexcept -> const Ping&;
    auto asPong() const& noexcept -> const Pong&;
    auto asReject() const& noexcept -> const Reject&;
    auto asSendaddr2() const& noexcept -> const Sendaddr2&;
    auto asTx() const& noexcept -> const Tx&;
    auto asVerack() const& noexcept -> const Verack&;
    auto asVersion() const& noexcept -> const Version&;
    auto Command() const noexcept -> message::Command;
    auto Describe() const noexcept -> ReadView;
    auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    auto Network() const noexcept -> opentxs::blockchain::Type;
    auto payload() const noexcept -> ByteArray;
    auto payload(Writer&&) const noexcept -> bool;
    auto Transmit(Transport type, zeromq::Message& out) const noexcept(false)
        -> void;

    auto asAddr() & noexcept -> Addr&;
    auto asAddr() && noexcept -> Addr;
    auto asAddr2() & noexcept -> Addr2&;
    auto asAddr2() && noexcept -> Addr2;
    auto asBlock() & noexcept -> Block&;
    auto asBlock() && noexcept -> Block;
    auto asCfcheckpt() & noexcept -> Cfcheckpt&;
    auto asCfcheckpt() && noexcept -> Cfcheckpt;
    auto asCfheaders() & noexcept -> Cfheaders&;
    auto asCfheaders() && noexcept -> Cfheaders;
    auto asCfilter() & noexcept -> Cfilter&;
    auto asCfilter() && noexcept -> Cfilter;
    auto asGetaddr() & noexcept -> Getaddr&;
    auto asGetaddr() && noexcept -> Getaddr;
    auto asGetblocks() & noexcept -> Getblocks&;
    auto asGetblocks() && noexcept -> Getblocks;
    auto asGetcfcheckpt() & noexcept -> Getcfcheckpt&;
    auto asGetcfcheckpt() && noexcept -> Getcfcheckpt;
    auto asGetcfheaders() & noexcept -> Getcfheaders&;
    auto asGetcfheaders() && noexcept -> Getcfheaders;
    auto asGetcfilters() & noexcept -> Getcfilters&;
    auto asGetcfilters() && noexcept -> Getcfilters;
    auto asGetdata() & noexcept -> Getdata&;
    auto asGetdata() && noexcept -> Getdata;
    auto asGetheaders() & noexcept -> Getheaders&;
    auto asGetheaders() && noexcept -> Getheaders;
    auto asHeaders() & noexcept -> Headers&;
    auto asHeaders() && noexcept -> Headers;
    auto asInv() & noexcept -> Inv&;
    auto asInv() && noexcept -> Inv;
    auto asMempool() & noexcept -> Mempool&;
    auto asMempool() && noexcept -> Mempool;
    auto asNotfound() & noexcept -> Notfound&;
    auto asNotfound() && noexcept -> Notfound;
    auto asPing() & noexcept -> Ping&;
    auto asPing() && noexcept -> Ping;
    auto asPong() & noexcept -> Pong&;
    auto asPong() && noexcept -> Pong;
    auto asReject() & noexcept -> Reject&;
    auto asReject() && noexcept -> Reject;
    auto asSendaddr2() & noexcept -> Sendaddr2&;
    auto asSendaddr2() && noexcept -> Sendaddr2;
    auto asTx() & noexcept -> Tx&;
    auto asTx() && noexcept -> Tx;
    auto asVerack() & noexcept -> Verack&;
    auto asVerack() && noexcept -> Verack;
    auto asVersion() & noexcept -> Version&;
    auto asVersion() && noexcept -> Version;
    auto swap(Message& rhs) noexcept -> void;

    Message(MessagePrivate* imp) noexcept;
    Message(allocator_type alloc = {}) noexcept;
    Message(const Message& rhs, allocator_type alloc = {}) noexcept;
    Message(Message&& rhs) noexcept;
    Message(Message&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Message& rhs) noexcept -> Message&;
    auto operator=(Message&& rhs) noexcept -> Message&;

    ~Message() override;

protected:
    friend MessagePrivate;

    MessagePrivate* imp_;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
