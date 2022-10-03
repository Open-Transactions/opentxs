// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Cfilter.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PCfilter(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto raw = ReturnType::BitcoinFormat{};
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (begin)")
            .Flush();

        return nullptr;
    }

    const auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    std::advance(it, sizeof(raw));
    expectedSize += 1;

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto filterSize = 0_uz;
    const auto haveSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, filterSize);

    if (false == haveSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    if ((expectedSize + filterSize) > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (filter)")
            .Flush();

        return nullptr;
    }

    const auto filterType = raw.Type(header.Network());

    try {
        const auto [elementCount, filterBytes] =
            blockchain::internal::DecodeSerializedCfilter(
                ReadView{reinterpret_cast<const char*>(it), filterSize});
        const auto* const start =
            reinterpret_cast<const std::byte*>(filterBytes.data());
        const auto* const end = start + filterBytes.size();

        return new ReturnType(
            api,
            std::move(pHeader),
            filterType,
            raw.Hash(),
            elementCount,
            Space{start, end});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BitcoinP2PCfilter(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::GCS& filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    return new ReturnType(api, network, type, hash, filter.ElementCount(), [&] {
        auto out = Space{};
        filter.Compressed(writer(out));

        return out;
    }());
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Cfilter::Cfilter(
    const api::Session& api,
    const blockchain::Type network,
    const cfilter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    const Space& compressed) noexcept
    : Message(api, network, bitcoin::Command::cfilter)
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(compressed)
    , params_(blockchain::internal::GetFilterParams(type_))
{
    init_hash();
}

Cfilter::Cfilter(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const cfilter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    Space&& compressed) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(std::move(compressed))
    , params_(blockchain::internal::GetFilterParams(type_))
{
}

auto Cfilter::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        const auto payload = [&] {
            auto filter = [&] {
                auto output = CompactSize(count_).Encode();
                output.insert(output.end(), filter_.begin(), filter_.end());

                return output;
            }();
            auto output = CompactSize(filter.size()).Encode();
            output.reserve(output.size() + filter.size());
            std::move(filter.begin(), filter.end(), std::back_inserter(output));

            return output;
        }();
        static constexpr auto fixed = sizeof(BitcoinFormat);
        const auto bytes = fixed + payload.size();

        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = BitcoinFormat{header().Network(), type_, hash_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), fixed);
        std::advance(i, fixed);
        std::memcpy(i, payload.data(), payload.size());
        std::advance(i, payload.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
