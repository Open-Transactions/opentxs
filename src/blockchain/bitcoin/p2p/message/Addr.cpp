// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/p2p/message/Addr.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PAddr(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Addr*
{
    try {
        namespace p2p = blockchain::p2p;
        namespace bitcoin = p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Addr;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto& header = *pHeader;
        const auto count = decode_compact_size(bytes, "address count");
        auto addresses = ReturnType::AddressVector{};
        const auto chain = header.Network();

        for (auto i = 0_uz; i < count; ++i) {
            const bool timestamp = ReturnType::SerializeTimestamp(version);

            if (timestamp) {
                auto raw = ReturnType::BitcoinFormat_31402{};
                deserialize_object(bytes, raw, "address");
                const auto [network, bytearray] =
                    ReturnType::ExtractAddress(raw.data_.address_);
                addresses.emplace_back(factory::BlockchainAddress(
                    api,
                    p2p::Protocol::bitcoin,
                    network,
                    bytearray.Bytes(),
                    raw.data_.port_.value(),
                    chain,
                    convert_stime(raw.time_.value()),
                    bitcoin::TranslateServices(
                        chain,
                        version,
                        bitcoin::GetServices(raw.data_.services_.value())),
                    false));
            } else {
                auto raw = bitcoin::AddressVersion{};
                deserialize_object(bytes, raw, "address");
                const auto [network, bytearray] =
                    ReturnType::ExtractAddress(raw.address_);
                addresses.emplace_back(factory::BlockchainAddress(
                    api,
                    p2p::Protocol::bitcoin,
                    network,
                    bytearray.Bytes(),
                    raw.port_.value(),
                    chain,
                    Time{},
                    bitcoin::TranslateServices(
                        chain,
                        version,
                        bitcoin::GetServices(raw.services_.value())),
                    false));
            }
        }

        check_finished(bytes);

        return new ReturnType(
            api, std::move(pHeader), version, std::move(addresses));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PAddr(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    UnallocatedVector<blockchain::p2p::Address>&& addresses)
    -> blockchain::p2p::bitcoin::message::internal::Addr*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Addr;

    return new ReturnType(api, network, version, std::move(addresses));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Addr::Addr(
    const api::Session& api,
    const blockchain::Type network,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, network, bitcoin::Command::addr)
    , version_(version)
    , payload_(std::move(addresses))
{
    init_hash();
}

Addr::Addr(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(addresses))
{
}

Addr::BitcoinFormat_31402::BitcoinFormat_31402(
    const blockchain::Type chain,
    const ProtocolVersion version,
    const p2p::Address& address)
    : time_(shorten(Clock::to_time_t(address.LastConnected())))
    , data_(chain, version, address)
{
}

Addr::BitcoinFormat_31402::BitcoinFormat_31402()
    : time_()
    , data_()
{
}

auto Addr::ExtractAddress(AddressByteField in) noexcept
    -> std::pair<p2p::Network, ByteArray>
{
    std::pair<p2p::Network, ByteArray> output{
        Network::ipv6, ByteArray{in.data(), in.size()}};
    auto& [type, bytes] = output;
    auto prefix = ByteArray{};

    if (bytes.Extract(AddressVersion::cjdns_prefix().size(), prefix) &&
        AddressVersion::cjdns_prefix() == prefix) {
        type = Network::cjdns;
    } else if (
        bytes.Extract(AddressVersion::ipv4_prefix().size(), prefix) &&
        AddressVersion::ipv4_prefix() == prefix) {
        type = Network::ipv4;
        bytes.Extract(
            bytes.size() - AddressVersion::ipv4_prefix().size(),
            prefix,
            AddressVersion::ipv4_prefix().size());
        bytes = prefix;

        OT_ASSERT(4 == bytes.size());
    } else if (
        bytes.Extract(AddressVersion::onion_prefix().size(), prefix) &&
        AddressVersion::onion_prefix() == prefix) {
        type = Network::onion2;
        bytes.Extract(
            bytes.size() - AddressVersion::onion_prefix().size(),
            prefix,
            AddressVersion::onion_prefix().size());
        bytes = prefix;

        OT_ASSERT(10 == bytes.size());
    }

    return output;
}

auto Addr::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto cs = CompactSize(payload_.size()).Encode();
        const auto bytes = [&] {
            const auto size = [this] {
                if (SerializeTimestamp()) {

                    return sizeof(BitcoinFormat_31402);
                } else {

                    return sizeof(AddressVersion);
                }
            }();

            return cs.size() + (payload_.size() * size);
        }();
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& address : payload_) {
            if (SerializeTimestamp()) {
                const auto raw =
                    BitcoinFormat_31402{header_->Network(), version_, address};
                std::memcpy(i, static_cast<const void*>(&raw), sizeof(raw));
                std::advance(i, sizeof(raw));
            } else {
                const auto raw =
                    AddressVersion{header_->Network(), version_, address};
                std::memcpy(i, static_cast<const void*>(&raw), sizeof(raw));
                std::advance(i, sizeof(raw));
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Addr::SerializeTimestamp(const ProtocolVersion version) noexcept -> bool
{
    return version >= 31402;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
