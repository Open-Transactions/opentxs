// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
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
namespace bitcoin
{
class Inventory;
}  // namespace bitcoin

namespace block
{
class Header;
}  // namespace block

namespace cfilter
{
class Hash;
}  // namespace cfilter

class GCS;
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
}  // namespace bitcoin
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
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
    const blockchain::GCS& filter,
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
    std::span<blockchain::bitcoin::Inventory> payload,
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
    std::span<blockchain::bitcoin::Inventory> payload,
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
auto BitcoinP2PNotfound(
    const api::Session& api,
    const blockchain::Type chain,
    std::span<blockchain::bitcoin::Inventory> payload,
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
    const network::blockchain::Transport style,
    const std::int32_t version,
    const Set<network::blockchain::bitcoin::Service>& localServices,
    const std::string_view localAddress,
    const std::uint16_t localPort,
    const Set<network::blockchain::bitcoin::Service>& remoteServices,
    const std::string_view remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool bip37,
    alloc::Default alloc) noexcept
    -> network::blockchain::bitcoin::message::internal::Version;
}  // namespace opentxs::factory
