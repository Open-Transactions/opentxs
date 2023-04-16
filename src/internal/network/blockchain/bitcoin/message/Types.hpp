// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "BoostAsio.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Data;
class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;
namespace ba = boost::asio;
namespace ip = ba::ip;
using tcp = ip::tcp;

namespace opentxs::network::blockchain::bitcoin::message
{
using BitVector8 = std::uint64_t;
using Nonce = std::uint64_t;
using ServiceBits = std::uint64_t;
using TxnCount = std::uint32_t;
using ProtocolVersion = std::int32_t;
using ProtocolVersionUnsigned = std::uint32_t;

using AddressByteField = ba::ip::address_v6::bytes_type;
using BitVectorField = be::little_uint64_buf_t;
using BlockHeaderField = std::array<std::byte, 80>;
using BlockHeaderHashField = std::array<std::byte, 32>;
using ChecksumField = std::array<std::byte, 4>;
using CommandField = std::array<std::byte, 12>;
using HeightField = be::little_int32_buf_t;
using MagicField = be::little_uint32_buf_t;
using MerkleBlockFlagsField = be::little_uint8_buf_t;
using NonceField = be::little_uint64_buf_t;
using PayloadSizeField = be::little_uint32_buf_t;
using PortField = be::big_uint16_buf_t;
using ProtocolVersionField = be::little_uint32_buf_t;
using ProtocolVersionFieldSigned = be::little_int32_buf_t;
using TimestampField32 = be::little_uint32_buf_t;
using TimestampField64 = be::little_int64_buf_t;
using TxnCountField = be::little_uint32_buf_t;
using Services64 = be::little_uint64_buf_t;

enum class Command : int {
    unknown = 0,
    addr,
    addr2,
    alert,
    avahello,
    block,
    blocktxn,
    cfcheckpt,
    cfheaders,
    cfilter,
    checkorder,
    cmpctblock,
    feefilter,
    filteradd,
    filterclear,
    filterload,
    getaddr,
    getblocks,
    getblocktxn,
    getcfcheckpt,
    getcfheaders,
    getcfilters,
    getdata,
    getheaders,
    headers,
    inv,
    mempool,
    merkleblock,
    notfound,
    ping,
    pong,
    protoconf,
    reject,
    reply,
    sendaddr2,
    sendcmpct,
    sendheaders,
    submitorder,
    tx,
    verack,
    version,
    xversion,
};  // IWYU pragma: export

enum class RejectCode : std::uint8_t {
    None = 0x00,
    DecodeFailed = 0x01,
    Invalid = 0x10,
    Obsolete = 0x11,
    Duplicate = 0x12,
    NonStandard = 0x40,
    DustThreshold = 0x41,
    LowPriority = 0x42,
    WrongChain = 0x43,
};  // IWYU pragma: export

enum class Service : std::uint8_t {
    None = 0,
    Bit1 = 1,
    Bit2 = 2,
    Bit3 = 3,
    Bit4 = 4,
    Bit5 = 5,
    Bit6 = 6,
    Bit7 = 7,
    Bit8 = 8,
    Bit9 = 9,
    Bit10 = 10,
    Bit11 = 11,
    Bit12 = 12,
    Bit13 = 13,
    Bit14 = 14,
    Bit15 = 15,
    Bit16 = 16,
    Bit17 = 17,
    Bit18 = 18,
    Bit19 = 19,
    Bit20 = 20,
    Bit21 = 21,
    Bit22 = 22,
    Bit23 = 23,
    Bit24 = 24,
    Bit25 = 25,
    Bit26 = 26,
    Bit27 = 27,
    Bit28 = 28,
    Bit29 = 29,
    Bit30 = 30,
    Bit31 = 31,
    Bit32 = 32,
    Bit33 = 33,
    Bit34 = 34,
    Bit35 = 35,
    Bit36 = 36,
    Bit37 = 37,
    Bit38 = 38,
    Bit39 = 39,
    Bit40 = 40,
    Bit41 = 41,
    Bit42 = 42,
    Bit43 = 43,
    Bit44 = 44,
    Bit45 = 45,
    Bit46 = 46,
    Bit47 = 47,
    Bit48 = 48,
    Bit49 = 49,
    Bit50 = 50,
    Bit51 = 51,
    Bit52 = 52,
    Bit53 = 53,
    Bit54 = 54,
    Bit55 = 55,
    Bit56 = 56,
    Bit57 = 57,
    Bit58 = 58,
    Bit59 = 59,
    Bit60 = 60,
    Bit61 = 61,
    Bit62 = 62,
    Bit63 = 63,
    Bit64 = 64,
};  // IWYU pragma: export

struct AddressVersion {
    static auto cjdns_prefix() -> ByteArray;
    static auto ipv4_prefix() -> ByteArray;
    static auto onion_prefix() -> ByteArray;

    BitVectorField services_;
    AddressByteField address_;
    PortField port_;

    static auto Encode(
        const network::blockchain::Transport type,
        const Data& bytes) -> AddressByteField;

    AddressVersion(
        const UnallocatedSet<message::Service>& services,
        const tcp::endpoint& endpoint) noexcept;
    AddressVersion(
        const opentxs::blockchain::Type chain,
        const ProtocolVersion version,
        const network::blockchain::Address& address) noexcept;
    AddressVersion() noexcept;
};

class Bip155
{
public:
    TimestampField32 time_;
    CompactSize services_;
    std::uint8_t network_id_;
    ByteArray addr_;
    CompactSize bytes_;
    PortField port_;

    static auto Decode(ReadView& bytes) noexcept(false) -> Bip155;
    static auto GetNetwork(std::uint8_t type, std::size_t addr) noexcept
        -> network::blockchain::Transport;

    auto GetNetwork() const noexcept -> network::blockchain::Transport;
    auto Serialize(WriteBuffer& out) const noexcept -> bool;
    auto size() const noexcept -> std::size_t;
    auto ToAddress(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        const ProtocolVersion version) const noexcept
        -> network::blockchain::Address;

    Bip155(
        const opentxs::blockchain::Type chain,
        const ProtocolVersion version,
        const network::blockchain::Address& address) noexcept;

private:
    Bip155() noexcept;
};

auto BitcoinString(const UnallocatedCString& in) noexcept -> ByteArray;
auto GetCommand(const CommandField& bytes) noexcept -> Command;
auto GetCommand(const ReadView bytes) noexcept -> Command;
auto GetServiceBytes(const UnallocatedSet<Service>& services) noexcept
    -> BitVector8;
auto GetServices(const BitVector8 data) noexcept -> UnallocatedSet<Service>;
auto print(const Command command) noexcept -> std::string_view;
auto SerializeCommand(const Command command) noexcept -> CommandField;
auto SerializeCommand(
    const Command command,
    network::zeromq::Message& out) noexcept -> void;
auto TranslateServices(
    const opentxs::blockchain::Type chain,
    const ProtocolVersion version,
    const Set<bitcoin::Service>& input) noexcept
    -> UnallocatedSet<message::Service>;  // TODO allocator
auto TranslateServices(
    const opentxs::blockchain::Type chain,
    const ProtocolVersion version,
    const UnallocatedSet<message::Service>& input) noexcept
    -> Set<bitcoin::Service>;  // TODO allocator

auto convert_service_bit(BitVector8 value) noexcept -> message::Service;
auto convert_service_bit(const message::Service value) noexcept -> BitVector8;
}  // namespace opentxs::network::blockchain::bitcoin::message

namespace opentxs::network::blockchain::bitcoin::message
{
using ClientFilterTypeField = be::little_uint8_buf_t;
using HashField = std::array<std::byte, 32>;
using InventoryTypeField = be::little_uint32_buf_t;

struct FilterPrefixBasic {
    ClientFilterTypeField type_;
    HashField hash_;

    auto Hash() const noexcept -> opentxs::blockchain::block::Hash;
    auto Type(const opentxs::blockchain::Type chain) const noexcept
        -> opentxs::blockchain::cfilter::Type;

    FilterPrefixBasic(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& hash) noexcept(false);
    FilterPrefixBasic() noexcept;
};
struct FilterPrefixChained {
    ClientFilterTypeField type_;
    HashField hash_;
    HashField previous_;

    auto Previous() const noexcept -> opentxs::blockchain::cfilter::Header;
    auto Stop() const noexcept -> opentxs::blockchain::block::Hash;
    auto Type(const opentxs::blockchain::Type chain) const noexcept
        -> opentxs::blockchain::cfilter::Type;

    FilterPrefixChained(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& stop,
        const opentxs::blockchain::cfilter::Header& prefix) noexcept(false);
    FilterPrefixChained() noexcept;
};
struct FilterRequest {
    ClientFilterTypeField type_;
    HeightField start_;
    HashField stop_;

    auto Start() const noexcept -> opentxs::blockchain::block::Height;
    auto Stop() const noexcept -> opentxs::blockchain::block::Hash;
    auto Type(const opentxs::blockchain::Type chain) const noexcept
        -> opentxs::blockchain::cfilter::Type;

    FilterRequest(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Height start,
        const opentxs::blockchain::block::Hash& stop) noexcept(false);
    FilterRequest() noexcept;
};
}  // namespace opentxs::network::blockchain::bitcoin::message
