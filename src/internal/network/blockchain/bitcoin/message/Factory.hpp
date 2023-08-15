// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace cfilter
{
class GCS;
class Hash;
}  // namespace cfilter
}  // namespace blockchain

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
class Cfheaders;
class Cfilter;
class Getaddr;
class Getcfheaders;
class Getcfilters;
class Getdata;
class Getheaders;
class Headers;
class Inv;
class Mempool;
class Notfound;
class Ping;
class Pong;
class Sendaddr2;
class Tx;
class Verack;
class Version;
}  // namespace internal
}  // namespace message

class Inventory;
}  // namespace bitcoin

class Address;
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto BitcoinP2PAddr(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    std::span<network::blockchain::Address> addresses,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Addr;
auto BitcoinP2PAddr2(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    std::span<network::blockchain::Address> addresses,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Addr2;
auto BitcoinP2PBlock(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView block,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Block;
auto BitcoinP2PCfheaders(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop,
    const blockchain::cfilter::Header& previous,
    std::span<blockchain::cfilter::Hash> hashes,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Cfheaders;
auto BitcoinP2PCfilter(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::cfilter::GCS& filter,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Cfilter;
auto BitcoinP2PGetaddr(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getaddr;
auto BitcoinP2PGetcfheaders(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getcfheaders;
auto BitcoinP2PGetcfilters(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::cfilter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getcfilters;
auto BitcoinP2PGetdata(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<network::blockchain::bitcoin::Inventory> payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getdata;
auto BitcoinP2PGetheaders(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersionUnsigned
        version,
    std::span<blockchain::block::Hash> history,
    const blockchain::block::Hash& stop,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Getheaders;
auto BitcoinP2PHeaders(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<blockchain::block::Header> headers,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Headers;
auto BitcoinP2PInv(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<network::blockchain::bitcoin::Inventory> payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Inv;
auto BitcoinP2PMempool(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Mempool;
auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    network::zeromq::Message&& incoming,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message;
auto BitcoinP2PMessage(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    ReadView header,
    ReadView payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message;
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
    -> network::blockchain::bitcoin::message::internal::Message;
auto BitcoinP2PMessageZMQ(
    const api::Session& api,
    const blockchain::Type chain,
    const network::blockchain::Transport type,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    network::zeromq::Message&& incoming,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Message;
auto BitcoinP2PNotfound(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<network::blockchain::bitcoin::Inventory> payload,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Notfound;
auto BitcoinP2PPing(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint64_t nonce,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Ping;
auto BitcoinP2PPong(
    const api::Session& api,
    const blockchain::Type chain,
    const std::uint64_t nonce,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Pong;
auto BitcoinP2PSendaddr2(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Sendaddr2;
auto BitcoinP2PTx(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView transaction,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Tx;
auto BitcoinP2PVerack(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Verack;
auto BitcoinP2PVersion(
    const api::Session& api,
    const blockchain::Type chain,
    const std::int32_t version,
    const network::blockchain::Address& localAddress,
    const network::blockchain::Address& remoteAddress,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool bip37,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Version;
}  // namespace opentxs::factory
