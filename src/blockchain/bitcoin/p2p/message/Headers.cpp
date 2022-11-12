// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Headers.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "internal/blockchain/block/Block.hpp"           // IWYU pragma: keep
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PHeaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Headers*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    const auto* it{static_cast<const std::byte*>(payload)};
    auto count = 0_uz;
    const bool decodedSize =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == decodedSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    UnallocatedVector<std::unique_ptr<blockchain::bitcoin::block::Header>>
        headers{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += 81;

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Block Header entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            auto pHeader = factory::BitcoinBlockHeader(
                api.Crypto(),
                header.Network(),
                ReadView{reinterpret_cast<const char*>(it), 80});

            if (pHeader) {
                headers.emplace_back(std::move(pHeader));

                it += 81;
            } else {
                LogError()("opentxs::factory::")(__func__)(
                    ": Invalid header received at index ")(i)
                    .Flush();

                break;
            }
        }
    }

    return new ReturnType(api, std::move(pHeader), std::move(headers));
}

auto BitcoinP2PHeaders(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<std::unique_ptr<blockchain::bitcoin::block::Header>>&&
        headers) -> blockchain::p2p::bitcoin::message::internal::Headers*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    return new ReturnType(api, network, std::move(headers));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Headers::Headers(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, network, bitcoin::Command::headers)
    , payload_(std::move(headers))
{
    init_hash();
}

Headers::Headers(
    const api::Session& api,
    std::unique_ptr<Header> header,
    UnallocatedVector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(headers))
{
}

auto Headers::payload(Writer&& out) const noexcept -> bool
{
    static constexpr auto null = std::byte{0x0};

    try {
        static constexpr auto length = 80_uz;
        const auto headers = payload_.size();
        const auto cs = CompactSize(headers).Encode();
        const auto bytes = cs.size() + (headers * (length + sizeof(null)));
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& pHeader : payload_) {
            OT_ASSERT(pHeader);

            const auto& header = *pHeader;

            if (false == header.Serialize(preallocated(length, i))) {
                throw std::runtime_error{"failed to serialize header"};
            }

            std::advance(i, length);
            std::memcpy(i, &null, sizeof(null));
            std::advance(i, sizeof(null));
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
