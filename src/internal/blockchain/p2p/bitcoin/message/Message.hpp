// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/bitcoin/bloom/BloomFilter.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
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
namespace block
{
class Header;
}  // namespace block

class Inventory;
}  // namespace bitcoin

namespace block
{
class Header;
class Transaction;
}  // namespace block

namespace p2p
{
namespace bitcoin
{
namespace message
{
class Cmpctblock;
class Feefilter;
class Getblocks;
class Getblocktxn;
class Merkleblock;
class Reject;
class Sendcmpct;
class Tx;
}  // namespace message

class Header;
}  // namespace bitcoin

class Address;
}  // namespace p2p

class GCS;
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::bitcoin
{
struct Message {
    static auto MaxPayload() -> std::size_t;

    virtual auto Encode() const -> ByteArray = 0;
    virtual auto header() const noexcept -> const Header& = 0;
    virtual auto payload() const noexcept -> ByteArray = 0;
    virtual auto payload(Writer&&) const noexcept -> bool = 0;
    virtual auto Transmit() const noexcept
        -> std::pair<zmq::Frame, zmq::Frame> = 0;

    virtual ~Message() = default;
};
}  // namespace opentxs::blockchain::p2p::bitcoin

namespace opentxs::blockchain::p2p::bitcoin::message
{
using ClientFilterTypeField = be::little_uint8_buf_t;
using HashField = std::array<std::byte, 32>;
using InventoryTypeField = be::little_uint32_buf_t;

struct FilterPrefixBasic {
    ClientFilterTypeField type_;
    HashField hash_;

    auto Hash() const noexcept -> block::Hash;
    auto Type(const blockchain::Type chain) const noexcept -> cfilter::Type;

    FilterPrefixBasic(
        const blockchain::Type chain,
        const cfilter::Type type,
        const block::Hash& hash) noexcept(false);
    FilterPrefixBasic() noexcept;
};
struct FilterPrefixChained {
    ClientFilterTypeField type_;
    HashField hash_;
    HashField previous_;

    auto Previous() const noexcept -> cfilter::Header;
    auto Stop() const noexcept -> block::Hash;
    auto Type(const blockchain::Type chain) const noexcept -> cfilter::Type;

    FilterPrefixChained(
        const blockchain::Type chain,
        const cfilter::Type type,
        const block::Hash& stop,
        const cfilter::Header& prefix) noexcept(false);
    FilterPrefixChained() noexcept;
};
struct FilterRequest {
    ClientFilterTypeField type_;
    HeightField start_;
    HashField stop_;

    auto Start() const noexcept -> block::Height;
    auto Stop() const noexcept -> block::Hash;
    auto Type(const blockchain::Type chain) const noexcept -> cfilter::Type;

    FilterRequest(
        const blockchain::Type chain,
        const cfilter::Type type,
        const block::Height start,
        const block::Hash& stop) noexcept(false);
    FilterRequest() noexcept;
};

auto VerifyChecksum(
    const api::Session& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept -> bool;
}  // namespace opentxs::blockchain::p2p::bitcoin::message

namespace opentxs::blockchain::p2p::bitcoin::message::internal
{
struct Addr : virtual public bitcoin::Message {
    using value_type = p2p::Address;
    using const_iterator =
        iterator::Bidirectional<const Addr, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Addr() override = default;
};
struct Addr2 : virtual public bitcoin::Message {
    using value_type = p2p::Address;
    using const_iterator =
        iterator::Bidirectional<const Addr2, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Addr2() override = default;
};
struct Block : virtual public bitcoin::Message {
    virtual auto GetBlock() const noexcept -> ByteArray = 0;

    ~Block() override = default;
};
struct Blocktxn : virtual public bitcoin::Message {
    virtual auto BlockTransactions() const noexcept -> ByteArray = 0;

    ~Blocktxn() override = default;
};
struct Cfcheckpt : virtual public bitcoin::Message {
    using value_type = cfilter::Header;
    using const_iterator =
        iterator::Bidirectional<const Cfcheckpt, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Cfcheckpt() override = default;
};
struct Cfheaders : virtual public bitcoin::Message {
    using value_type = cfilter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto Previous() const noexcept -> const cfilter::Header& = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Cfheaders() override = default;
};
struct Cfilter : virtual public bitcoin::Message {
    virtual auto Bits() const noexcept -> std::uint8_t = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    virtual auto FPRate() const noexcept -> std::uint32_t = 0;
    virtual auto Filter() const noexcept -> ReadView = 0;
    virtual auto Hash() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Cfilter() override = default;
};
struct Filteradd : virtual public bitcoin::Message {
    virtual auto Element() const noexcept -> ByteArray = 0;

    ~Filteradd() override = default;
};
struct Filterclear : virtual public bitcoin::Message {

    ~Filterclear() override = default;
};
struct Filterload : virtual public bitcoin::Message {
    virtual auto Filter() const noexcept -> OTBloomFilter = 0;

    ~Filterload() override = default;
};
struct Getaddr : virtual public bitcoin::Message {

    ~Getaddr() override = default;
};
struct Getcfcheckpt : virtual public bitcoin::Message {
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Getcfcheckpt() override = default;
};
struct Getcfheaders : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Getcfheaders() override = default;
};
struct Getcfilters : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> cfilter::Type = 0;

    ~Getcfilters() override = default;
};
struct Getdata : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Getdata, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Getdata() override = default;
};
struct Getheaders : virtual public bitcoin::Message {
    using value_type = block::Hash;
    using const_iterator =
        iterator::Bidirectional<const Getheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto StopHash() const noexcept -> block::Hash = 0;
    virtual auto Version() const noexcept -> ProtocolVersionUnsigned = 0;

    ~Getheaders() override = default;
};
struct Headers : virtual public bitcoin::Message {
    using value_type = blockchain::block::Header;

    virtual auto get() const noexcept -> std::span<const value_type> = 0;

    virtual auto get() noexcept -> std::span<value_type> = 0;

    ~Headers() override = default;
};
struct Inv : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator = iterator::Bidirectional<const Inv, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Inv() override = default;
};
struct Mempool : virtual public bitcoin::Message {

    ~Mempool() override = default;
};
struct Notfound : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Notfound, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Notfound() override = default;
};
struct Ping : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;

    ~Ping() override = default;
};
struct Pong : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;

    ~Pong() override = default;
};
struct Sendaddr2 : virtual public bitcoin::Message {

    ~Sendaddr2() override = default;
};
struct Sendheaders : virtual public bitcoin::Message {

    ~Sendheaders() override = default;
};
struct Tx : virtual public bitcoin::Message {
    virtual auto Transaction(alloc::Default alloc) const noexcept
        -> blockchain::block::Transaction = 0;

    ~Tx() override = default;
};
struct Verack : virtual public bitcoin::Message {

    ~Verack() override = default;
};
struct Version : virtual public bitcoin::Message {
    virtual auto Height() const noexcept -> block::Height = 0;
    virtual auto LocalAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto LocalServices() const noexcept
        -> UnallocatedSet<blockchain::p2p::Service> = 0;
    virtual auto Nonce() const noexcept -> p2p::bitcoin::Nonce = 0;
    virtual auto ProtocolVersion() const noexcept
        -> bitcoin::ProtocolVersion = 0;
    virtual auto Relay() const noexcept -> bool = 0;
    virtual auto RemoteAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto RemoteServices() const noexcept
        -> UnallocatedSet<blockchain::p2p::Service> = 0;
    virtual auto UserAgent() const noexcept -> const UnallocatedCString& = 0;

    ~Version() override = default;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::internal

namespace opentxs::factory
{
auto BitcoinP2PAddr(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Addr*;
auto BitcoinP2PAddr(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    UnallocatedVector<blockchain::p2p::Address>&& addresses)
    -> blockchain::p2p::bitcoin::message::internal::Addr*;
auto BitcoinP2PAddr2(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Addr2>;
auto BitcoinP2PAddr2(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::span<blockchain::p2p::Address> addresses) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Addr2>;
auto BitcoinP2PBlock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Block*;
auto BitcoinP2PBlock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_block)
    -> blockchain::p2p::bitcoin::message::internal::Block*;
auto BitcoinP2PBlocktxn(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Blocktxn*;
auto BitcoinP2PBlocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_Blocktxn)
    -> blockchain::p2p::bitcoin::message::internal::Blocktxn*;
auto BitcoinP2PCfcheckpt(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*;
auto BitcoinP2PCfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop,
    Vector<blockchain::cfilter::Header>&& headers)
    -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*;
auto BitcoinP2PCfheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Cfheaders*;
auto BitcoinP2PCfheaders(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop,
    const blockchain::cfilter::Header& previous,
    Vector<blockchain::cfilter::Hash>&& hashes)
    -> blockchain::p2p::bitcoin::message::internal::Cfheaders*;
auto BitcoinP2PCfilter(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Cfilter*;
auto BitcoinP2PCfilter(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::GCS& filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*;
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Cmpctblock*;
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
    -> blockchain::p2p::bitcoin::message::Cmpctblock*;
auto BitcoinP2PFeefilter(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Feefilter*;
auto BitcoinP2PFeefilter(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t fee_rate)
    -> blockchain::p2p::bitcoin::message::Feefilter*;
auto BitcoinP2PFilteradd(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Filteradd*;
auto BitcoinP2PFilteradd(
    const api::Session& api,
    const blockchain::Type network,
    const Data& element)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*;
auto BitcoinP2PFilterclear(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Filterclear*;
auto BitcoinP2PFilterclear(
    const api::Session& api,
    const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Filterclear*;
auto BitcoinP2PFilterload(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Filterload*;
auto BitcoinP2PFilterload(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::BloomFilter& filter)
    -> blockchain::p2p::bitcoin::message::internal::Filterload*;
auto BitcoinP2PGetaddr(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Getaddr*;
auto BitcoinP2PGetaddr(const api::Session& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Getaddr*;
auto BitcoinP2PGetblocks(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Getblocks*;
auto BitcoinP2PGetblocks(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint32_t version,
    Vector<blockchain::block::Hash>&& header_hashes,
    blockchain::block::Hash&& stop_hash)
    -> blockchain::p2p::bitcoin::message::Getblocks*;
auto BitcoinP2PGetblocktxn(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Getblocktxn*;
auto BitcoinP2PGetblocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block_hash,
    const UnallocatedVector<std::size_t>& txn_indices)
    -> blockchain::p2p::bitcoin::message::Getblocktxn*;
auto BitcoinP2PGetcfcheckpt(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*;
auto BitcoinP2PGetcfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*;
auto BitcoinP2PGetcfheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes)
    -> blockchain::p2p::bitcoin::message::internal::Getcfheaders*;
auto BitcoinP2PGetcfheaders(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfheaders*;
auto BitcoinP2PGetcfilters(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*;
auto BitcoinP2PGetcfilters(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*;
auto BitcoinP2PGetdata(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Getdata*;
auto BitcoinP2PGetdata(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Getdata*;
auto BitcoinP2PGetheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Getheaders*;
auto BitcoinP2PGetheaders(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersionUnsigned version,
    Vector<blockchain::block::Hash>&& history,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*;
auto BitcoinP2PHeaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Headers*;
auto BitcoinP2PHeaders(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::block::Header>&& headers)
    -> blockchain::p2p::bitcoin::message::internal::Headers*;
auto BitcoinP2PInv(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Inv>;
auto BitcoinP2PInv(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Inv*;
auto BitcoinP2PMempool(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Mempool*;
auto BitcoinP2PMempool(const api::Session& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Mempool*;
auto BitcoinP2PMerkleblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Merkleblock*;
auto BitcoinP2PNotfound(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Notfound*;
auto BitcoinP2PNotfound(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Notfound*;
auto BitcoinP2PPing(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Ping*;
auto BitcoinP2PPing(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t nonce)
    -> blockchain::p2p::bitcoin::message::internal::Ping*;
auto BitcoinP2PPong(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Pong*;
auto BitcoinP2PPong(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t nonce)
    -> blockchain::p2p::bitcoin::message::internal::Pong*;
auto BitcoinP2PReject(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Reject*;
auto BitcoinP2PReject(
    const api::Session& api,
    const blockchain::Type network,
    const UnallocatedCString& message,
    const std::uint8_t code,
    const UnallocatedCString& reason,
    const Data& extra) -> blockchain::p2p::bitcoin::message::Reject*;
auto BitcoinP2PSendaddr2(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Sendaddr2*;
auto BitcoinP2PSendaddr2(
    const api::Session& api,
    const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Sendaddr2*;
auto BitcoinP2PSendcmpct(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Sendcmpct*;
auto BitcoinP2PSendcmpct(
    const api::Session& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version)
    -> blockchain::p2p::bitcoin::message::Sendcmpct*;
auto BitcoinP2PSendheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Sendheaders*;
auto BitcoinP2PSendheaders(
    const api::Session& api,
    const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Sendheaders*;
auto BitcoinP2PTx(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>;
auto BitcoinP2PTx(
    const api::Session& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>;
auto BitcoinP2PVerack(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Verack*;
auto BitcoinP2PVerack(const api::Session& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Verack*;
auto BitcoinP2PVersion(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Version*;
auto BitcoinP2PVersion(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::Network style,
    const std::int32_t version,
    const UnallocatedSet<blockchain::p2p::Service>& localServices,
    const UnallocatedCString& localAddress,
    const std::uint16_t localPort,
    const UnallocatedSet<blockchain::p2p::Service>& remoteServices,
    const UnallocatedCString& remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool relay) -> blockchain::p2p::bitcoin::message::internal::Version*;
}  // namespace opentxs::factory

namespace opentxs::factory
{
// TODO templates in network/blockchain/bitcoin/Peer.tpp assume the raw pointer
// version of these functions but eventually we want all these factories to
// return unique_ptr. Once that conversion is complete and Peer.tpp is updated
// to use the updated versions then these temporary functions can be removed
auto BitcoinP2PAddr2Temp(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> blockchain::p2p::bitcoin::message::internal::Addr2*;
auto BitcoinP2PAddr2Temp(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::span<blockchain::p2p::Address> addresses) noexcept
    -> blockchain::p2p::bitcoin::message::internal::Addr2*;
auto BitcoinP2PInvTemp(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> blockchain::p2p::bitcoin::message::internal::Inv*;
auto BitcoinP2PTxTemp(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept
    -> blockchain::p2p::bitcoin::message::internal::Tx*;
}  // namespace opentxs::factory
