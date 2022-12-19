// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/p2p/message/Addr2.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PAddr2(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Addr2>
{
    using ReturnType = blockchain::p2p::bitcoin::message::implementation::Addr2;

    if (false == pHeader.operator bool()) {

        throw std::runtime_error{"invalid header"};
    }

    const auto& header = *pHeader;
    const auto count = decode_compact_size(bytes, "address count");
    constexpr auto maxAddresses = 1000_uz;

    if (maxAddresses < count) {
        const auto error =
            UnallocatedCString{"too many addresses in message: "}.append(
                std::to_string(count));

        throw std::runtime_error{error};
    }

    auto data = ReturnType::AddressVector{};  // TODO allocator
    data.reserve(count);

    for (auto i = 0_uz; i < count; ++i) {
        using namespace blockchain::p2p::bitcoin;
        auto bip155 = Bip155::Decode(bytes);
        data.emplace_back(bip155.ToAddress(api, header.Network(), version));
    }

    check_finished(bytes);

    return std::make_unique<ReturnType>(
        api, std::move(pHeader), version, std::move(data));
}

auto BitcoinP2PAddr2Temp(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) noexcept(false)
    -> blockchain::p2p::bitcoin::message::internal::Addr2*
{
    return BitcoinP2PAddr2(api, std::move(pHeader), version, bytes).release();
}

auto BitcoinP2PAddr2(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::span<blockchain::p2p::Address> in) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Addr2>
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Addr2;

    return std::make_unique<ReturnType>(api, network, version, [&] {
        auto out = ReturnType::AddressVector{};  // TODO allocator
        out.reserve(in.size());
        std::move(in.begin(), in.end(), std::back_inserter(out));

        return out;
    }());
}

auto BitcoinP2PAddr2Temp(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::span<blockchain::p2p::Address> addresses) noexcept
    -> blockchain::p2p::bitcoin::message::internal::Addr2*
{
    return BitcoinP2PAddr2(api, network, version, addresses).release();
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
using namespace std::literals;

Addr2::Addr2(
    const api::Session& api,
    const blockchain::Type network,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, network, bitcoin::Command::addr2)
    , version_(version)
    , payload_(std::move(addresses))
{
    init_hash();
}

Addr2::Addr2(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(addresses))
{
}

auto Addr2::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto cs = CompactSize(payload_.size());
        const auto [bytes, data] = [&, this] {
            auto v = Vector<Bip155>{};  // TODO allocator
            auto size = cs.Size();
            v.reserve(size);

            for (const auto& addr : payload_) {
                const auto& bip155 = v.emplace_back(version_, addr);
                size += bip155.size();
            }

            return std::make_pair(size, v);
        }();
        auto buf = out.Reserve(bytes);

        if (false == buf.IsValid(bytes)) {

            throw std::runtime_error{"failed to allocated space for output"};
        }

        serialize_compact_size(cs, buf, "address count"sv);

        for (const auto& addr : data) {
            if (false == addr.Serialize(buf)) {

                throw std::runtime_error{"failed to serialize addresst"};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
