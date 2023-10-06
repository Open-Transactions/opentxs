// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Types.hpp"  // IWYU pragma: associated

#include <ankerl/unordered_dense.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/blockchain/Factory.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::network::blockchain::bitcoin::message
{
using namespace std::literals;

static constexpr auto bip155_to_opentxs_ = [] {
    using enum network::blockchain::Transport;

    return frozen::
        make_unordered_map<std::uint8_t, network::blockchain::Transport>({
            {1, ipv4},
            {2, ipv6},
            {3, onion2},
            {4, onion3},
            {5, eep},
            {6, cjdns},
            {90, zmq},
        });
}();
static constexpr auto bip155_to_address_bytes_ = [] {
    return frozen::
        make_unordered_map<std::uint8_t, std::pair<std::size_t, std::size_t>>({
            {1, {4_uz, 4_uz}},
            {2, {16_uz, 16_uz}},
            {3, {10_uz, 10_uz}},
            {4, {32_uz, 32_uz}},
            {5, {32_uz, 32_uz}},
            {6, {16_uz, 16_uz}},
            {90, {10_uz, 252_uz}},
        });
}();
static constexpr auto opentxs_to_bip155_ =
    frozen::invert_unordered_map(bip155_to_opentxs_);

static constexpr auto command_map_ = [] {
    using enum Command;

    return frozen::make_unordered_map<Command, std::string_view>({
        {addr, "addr"},
        {addr2, "addrv2"},
        {alert, "alert"},
        {authch, "authch"},
        {avahello, "avahello"},
        {block, "block"},
        {blocktxn, "blocktxn"},
        {cfcheckpt, "cfcheckpt"},
        {cfheaders, "cfheaders"},
        {cfilter, "cfilter"},
        {checkorder, "checkorder"},
        {cmpctblock, "cmpctblock"},
        {feefilter, "feefilter"},
        {filteradd, "filteradd"},
        {filterclear, "filterclear"},
        {filterload, "filterload"},
        {getaddr, "getaddr"},
        {getblocks, "getblocks"},
        {getblocktxn, "getblocktxn"},
        {getcfcheckpt, "getcfcheckpt"},
        {getcfheaders, "getcfheaders"},
        {getcfilters, "getcfilters"},
        {getdata, "getdata"},
        {getheaders, "getheaders"},
        {headers, "headers"},
        {inv, "inv"},
        {mempool, "mempool"},
        {merkleblock, "merkleblock"},
        {notfound, "notfound"},
        {ping, "ping"},
        {pong, "pong"},
        {protoconf, "protoconf"},
        {reject, "reject"},
        {reply, "reply"},
        {sendaddr2, "sendaddrv2"},
        {sendcmpct, "sendcmpct"},
        {senddsq, "senddsq"},
        {sendheaders, "sendheaders"},
        {sendheaders2, "sendheaders2"},
        {submitorder, "submitorder"},
        {tx, "tx"},
        {verack, "verack"},
        {version, "version"},
        {xversion, "xversion"},
    });
}();

auto AddressVersion::cjdns_prefix() -> ByteArray
{
    static auto output = []() {
        auto out = ByteArray{};
        out.DecodeHex("0xfc");
        return out;
    }();
    return output;
}
auto AddressVersion::ipv4_prefix() -> ByteArray
{
    static auto output = []() {
        auto out = ByteArray{};
        out.DecodeHex("0x00000000000000000000ffff");
        return out;
    }();
    return output;
}
auto AddressVersion::onion_prefix() -> ByteArray
{
    static auto output = []() {
        auto out = ByteArray{};
        out.DecodeHex("0xfd87d87eeb43");
        return out;
    }();
    return output;
}

AddressVersion::AddressVersion(
    const UnallocatedSet<Service>& services,
    const tcp::endpoint& endpoint) noexcept
    : services_(GetServiceBytes(services))
    , address_(endpoint.address().to_v6().to_bytes())
    , port_(endpoint.port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion(
    const opentxs::blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    const network::blockchain::Address& address) noexcept
    : services_(GetServiceBytes(
          TranslateServices(chain, version, address.Services())))
    , address_(Encode(address.Type(), address.Bytes()))
    , port_(address.Port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion() noexcept
    : services_()
    , address_()
    , port_()
{
    static_assert(26 == sizeof(AddressVersion));
}

auto AddressVersion::Encode(
    const network::blockchain::Transport type,
    const Data& bytes) -> AddressByteField
{
    AddressByteField output{};

    switch (type) {
        case network::blockchain::Transport::ipv6:
        case network::blockchain::Transport::cjdns: {
            OT_ASSERT(output.size() == bytes.size());

            std::memcpy(output.data(), bytes.data(), output.size());
        } break;
        case network::blockchain::Transport::ipv4: {
            auto encoded{ipv4_prefix()};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded.size());

            std::memcpy(output.data(), encoded.data(), output.size());
        } break;
        case network::blockchain::Transport::onion2: {
            auto encoded{onion_prefix()};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded.size());

            std::memcpy(output.data(), encoded.data(), output.size());
        } break;
        case network::blockchain::Transport::onion3:
        case network::blockchain::Transport::eep:
        case network::blockchain::Transport::zmq:
        default: {
            OT_FAIL;
        }
    }

    return output;
}

Bip155::Bip155() noexcept
    : time_()
    , services_()
    , network_id_()
    , addr_()
    , bytes_()
    , port_()
{
}

Bip155::Bip155(
    const opentxs::blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    const network::blockchain::Address& address) noexcept
    : time_(shorten(Clock::to_time_t(address.LastConnected())))
    , services_(GetServiceBytes(
          TranslateServices(chain, version, address.Services())))
    , network_id_([&]() -> std::uint8_t {
        const auto& map = opentxs_to_bip155_;

        if (const auto* i = map.find(address.Type()); map.end() != i) {

            return i->second;
        } else {

            return 0;
        }
    }())
    , addr_([&] {
        using enum Transport;

        if (address.Type() == zmq) {
            const auto subtype = [&]() -> std::uint8_t {
                const auto& map = opentxs_to_bip155_;

                if (const auto* i = map.find(address.Subtype());
                    map.end() != i) {

                    return i->second;
                } else {

                    return 0;
                }
            }();
            const auto key = address.Key();
            const auto addr = address.Bytes();
            auto out = ByteArray{};
            auto buf = reserve(
                out.WriteInto(),
                key.size() + sizeof(subtype) + addr.size(),
                "zmq endpoint data");
            copy(key, buf, "key");
            serialize_object(subtype, buf, "subtype");
            copy(addr.Bytes(), buf, "addr");

            return out;
        } else {

            return address.Bytes();
        }
    }())
    , bytes_(addr_.size())
    , port_(address.Port())
{
}

auto Bip155::Decode(ReadView& in) noexcept(false) -> Bip155
{
    auto notUsed = ReadView{};
    auto out = Bip155{};
    auto& id = out.network_id_;
    deserialize_object(in, out.time_, "time field"sv);
    DecodeCompactSize(in, notUsed, std::addressof(out.services_));
    deserialize_object(in, id, "network id"sv);
    DecodeCompactSize(in, notUsed, std::addressof(out.bytes_));
    deserialize(
        in,
        out.addr_.WriteInto(),
        convert_to_size(out.bytes_.Value()),
        "address"sv);
    deserialize_object(in, out.port_, "port"sv);

    return out;
}

auto Bip155::GetNetwork(std::uint8_t in, std::size_t addr) noexcept
    -> network::blockchain::Transport
{
    using enum network::blockchain::Transport;
    const auto& conv = bip155_to_opentxs_;
    const auto& bytes = bip155_to_address_bytes_;

    if (const auto* i = conv.find(in); conv.end() != i) {
        if (const auto* j = bytes.find(in); bytes.end() != j) {
            const auto& [min, max] = j->second;

            if ((addr >= min) && (addr <= max)) { return i->second; }
        }
    }

    return invalid;
}

auto Bip155::GetNetwork() const noexcept -> network::blockchain::Transport
{
    return GetNetwork(network_id_, convert_to_size(bytes_.Value()));
}

auto Bip155::Serialize(WriteBuffer& out) const noexcept -> bool
{
    try {
        serialize_object(time_, out, "time"sv);
        serialize_compact_size(services_, out, "services"sv);
        serialize_object(network_id_, out, "network id"sv);
        serialize_compact_size(bytes_, out, "address bytes"sv);
        copy(addr_.Bytes(), out, "address");
        serialize_object(port_, out, "port"sv);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(": ")(e.what()).Flush();

        return false;
    }
}

auto Bip155::size() const noexcept -> std::size_t
{
    return sizeof(time_) + sizeof(network_id_) + sizeof(port_) +
           services_.Size() + bytes_.Total();
}

auto Bip155::ToAddress(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version)
    const noexcept -> network::blockchain::Address
{
    using enum Transport;
    using enum Protocol;
    const auto type = GetNetwork();

    if (zmq == type) {
        try {
            auto addr = addr_.Bytes();
            auto key = std::array<char, 32_uz>{};
            const auto keyView = ReadView{key.data(), key.size()};
            auto subtype = std::uint8_t{};
            deserialize(
                addr, preallocated(key.size(), key.data()), key.size(), "key");
            deserialize_object(addr, subtype, "subtype");

            return factory::BlockchainAddress(
                api,
                bitcoin,
                type,
                GetNetwork(subtype, addr.size()),
                keyView,
                addr,
                port_.value(),
                chain,
                convert_time(time_.value()),
                TranslateServices(
                    chain, version, GetServices(services_.Value())),
                false,
                {});
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(": ")(e.what()).Flush();

            return {};
        }
    }

    return factory::BlockchainAddress(
        api,
        bitcoin,
        type,
        network::blockchain::Transport::invalid,
        addr_.Bytes(),
        port_.value(),
        chain,
        convert_time(time_.value()),
        TranslateServices(chain, version, GetServices(services_.Value())),
        false,
        {});
}

auto BitcoinString(const UnallocatedCString& in) noexcept -> ByteArray
{
    auto output = CompactSize(in.size()).Encode();

    if (false == in.empty()) { output.Concatenate(in.data(), in.size()); }

    return output;
}

auto convert_service_bit(BitVector8 value) noexcept -> Service
{
    if (0 == value) { return Service::None; }

    auto log{1};

    while (value >>= 1) { ++log; }

    return static_cast<Service>(log);
}

auto convert_service_bit(const Service value) noexcept -> BitVector8
{
    if (Service::None == value) { return {}; }

    BitVector8 output{1u};
    output <<= (static_cast<std::uint8_t>(value) - 1);

    return output;
}

auto GetCommand(const CommandField& bytes) noexcept -> Command
{
    // NOTE this field is padded with null bytes
    const auto* p = reinterpret_cast<const char*>(bytes.data());

    return GetCommand(ReadView{p, std::strlen(p)});
}

auto GetCommand(const ReadView bytes) noexcept -> Command
{
    static const auto map = reverse_arbitrary_map<
        Command,
        std::string_view,
        ankerl::unordered_dense::map<std::string_view, Command>>(command_map_);

    try {
        const UnallocatedCString raw{
            reinterpret_cast<const char*>(bytes.data()), bytes.size()};
        const auto command = UnallocatedCString{raw.c_str()};

        return map.at(command);
    } catch (...) {

        return Command::unknown;
    }
}

auto GetServiceBytes(const UnallocatedSet<Service>& services) noexcept
    -> BitVector8
{
    BitVector8 output{0};

    for (const auto& bit : services) {
        if (Service::None == bit) { continue; }

        output |= convert_service_bit(bit);
    }

    return output;
}

#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
auto GetServices(
    const BitVector8 data) noexcept -> UnallocatedSet<Service>
{
    if (0 == data) { return {}; }

    auto output = UnallocatedSet<Service>{};
    auto mask = BitVector8{1};

    for (std::size_t i = 0; i < (8 * sizeof(data)); ++i) {
        const auto value = data & mask;

        if (0 != value) { output.emplace(convert_service_bit(value)); }

        // NOTE mask will overflow on the last iteration of the loop but it
        // doesn't matter since it will never be accessed again
        mask *= 2;
    }

    return output;
}
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

auto print(const Command in) noexcept -> std::string_view
{
    const auto& map = command_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return {};
    }
}

auto SerializeCommand(const Command command) noexcept -> CommandField
{
    CommandField output{};

    try {
        const auto string = print(command);

        OT_ASSERT(output.size() >= string.size());

        std::memcpy(output.data(), string.data(), string.size());
    } catch (...) {
    }

    return output;
}

auto SerializeCommand(
    const Command command,
    network::zeromq::Message& out) noexcept -> void
{
    const auto bytes = print(command);
    out.AddFrame(bytes.data(), bytes.size());
}

auto TranslateServices(
    const opentxs::blockchain::Type chain,
    [[maybe_unused]] const network::blockchain::bitcoin::message::
        ProtocolVersion version,
    const Set<bitcoin::Service>& input) noexcept
    -> UnallocatedSet<message::Service>
{
    auto output = UnallocatedSet<message::Service>{};

    try {
        const auto& data = opentxs::blockchain::params::get(chain);
        std::ranges::for_each(input, [&](const auto& in) -> void {
            if (auto value = data.TranslateService(in); value) {
                output.emplace(*value);
            }
        });
    } catch (const std::exception& e) {
        LogAbort()("opentxs::network::blockchain::bitcoin::message::")(
            __func__)(": ")(e.what())
            .Abort();
    }

    return output;
}

auto TranslateServices(
    const opentxs::blockchain::Type chain,
    [[maybe_unused]] const network::blockchain::bitcoin::message::
        ProtocolVersion version,
    const UnallocatedSet<message::Service>& input) noexcept
    -> Set<bitcoin::Service>
{
    auto output = Set<bitcoin::Service>{};  // TODO allocator
    output.clear();
    const auto& data = opentxs::blockchain::params::get(chain);
    std::ranges::for_each(input, [&](const auto& in) -> void {
        if (auto value = data.TranslateService(in); value) {
            output.emplace(*value);
        }
    });

    return output;
}
}  // namespace opentxs::network::blockchain::bitcoin::message

namespace opentxs::network::blockchain::bitcoin::message
{
FilterPrefixBasic::FilterPrefixBasic(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::cfilter::Type type,
    const opentxs::blockchain::block::Hash& hash) noexcept(false)
    : type_(opentxs::blockchain::internal::Serialize(chain, type))
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));

    if (hash.size() != hash_.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(hash_.data(), hash.data(), hash_.size());
}

FilterPrefixBasic::FilterPrefixBasic() noexcept
    : type_()
    , hash_()
{
    static_assert(33 == sizeof(FilterPrefixBasic));
}

auto FilterPrefixBasic::Hash() const noexcept
    -> opentxs::blockchain::block::Hash
{
    return ReadView{reinterpret_cast<const char*>(hash_.data()), hash_.size()};
}

auto FilterPrefixBasic::Type(const opentxs::blockchain::Type chain)
    const noexcept -> opentxs::blockchain::cfilter::Type
{
    return opentxs::blockchain::internal::Deserialize(chain, type_.value());
}

FilterPrefixChained::FilterPrefixChained(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::cfilter::Type type,
    const opentxs::blockchain::block::Hash& stop,
    const opentxs::blockchain::cfilter::Header& previous) noexcept(false)
    : type_(opentxs::blockchain::internal::Serialize(chain, type))
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));

    if (stop.size() != hash_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    if (previous.size() != previous_.size()) {
        throw std::runtime_error("Invalid previous hash");
    }

    std::memcpy(hash_.data(), stop.data(), hash_.size());
    std::memcpy(previous_.data(), previous.data(), previous_.size());
}

FilterPrefixChained::FilterPrefixChained() noexcept
    : type_()
    , hash_()
    , previous_()
{
    static_assert(65 == sizeof(FilterPrefixChained));
}

auto FilterPrefixChained::Previous() const noexcept
    -> opentxs::blockchain::cfilter::Header
{
    return ReadView{
        reinterpret_cast<const char*>(previous_.data()), previous_.size()};
}

auto FilterPrefixChained::Stop() const noexcept
    -> opentxs::blockchain::block::Hash
{
    return ReadView{reinterpret_cast<const char*>(hash_.data()), hash_.size()};
}

auto FilterPrefixChained::Type(const opentxs::blockchain::Type chain)
    const noexcept -> opentxs::blockchain::cfilter::Type
{
    return opentxs::blockchain::internal::Deserialize(chain, type_.value());
}

FilterRequest::FilterRequest(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::cfilter::Type type,
    const opentxs::blockchain::block::Height start,
    const opentxs::blockchain::block::Hash& stop) noexcept(false)
    : type_(opentxs::blockchain::internal::Serialize(chain, type))
    , start_(static_cast<std::uint32_t>(start))
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));
    static_assert(sizeof(std::uint32_t) == sizeof(start_));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= start);

    if (stop.size() != stop_.size()) {
        throw std::runtime_error("Invalid stop hash");
    }

    std::memcpy(stop_.data(), stop.data(), stop_.size());
}

FilterRequest::FilterRequest() noexcept
    : type_()
    , start_()
    , stop_()
{
    static_assert(37 == sizeof(FilterRequest));
}

auto FilterRequest::Start() const noexcept -> opentxs::blockchain::block::Height
{
    return start_.value();
}

auto FilterRequest::Stop() const noexcept -> opentxs::blockchain::block::Hash
{
    return ReadView{reinterpret_cast<const char*>(stop_.data()), stop_.size()};
}

auto FilterRequest::Type(const opentxs::blockchain::Type chain) const noexcept
    -> opentxs::blockchain::cfilter::Type
{
    return opentxs::blockchain::internal::Deserialize(chain, type_.value());
}
}  // namespace opentxs::network::blockchain::bitcoin::message
