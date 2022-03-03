// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Ping.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PPing(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Ping*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Ping;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    bitcoin::Nonce nonce{};
    auto expectedSize = sizeof(ReturnType::BitcoinFormat_60001);

    if (expectedSize <= size) {
        // Older Pings do not include the nonce field. That field was introduced
        // in protocol 60001. So we check to see if the size includes what's
        // expected for protocol 60001 (which FYI is a nonce field). If it is,
        // then we'll read the nonce. Otherwise we just assume there's no nonce
        // and return. We can't determine here if that's an error or not, since
        // we don't know the expected protocol version in this spot.
        auto* it{static_cast<const std::byte*>(payload)};
        ReturnType::BitcoinFormat_60001 raw{};
        std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
        it += sizeof(raw);
        nonce = raw.nonce_.value();
    } else {
        // Apparently there's no nonce field included. Must be a message from a
        // node running an older protocol version. This is still "success"
        // though, as far as I can tell.
    }

    return new ReturnType(api, std::move(pHeader), nonce);
}

auto BitcoinP2PPing(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t nonce)
    -> blockchain::p2p::bitcoin::message::internal::Ping*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Ping;

    return new ReturnType(api, network, nonce);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Ping::Ping(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::Nonce nonce) noexcept
    : Message(api, network, bitcoin::Command::ping)
    , nonce_(nonce)
{
    init_hash();
}

Ping::Ping(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::Nonce nonce) noexcept
    : Message(api, std::move(header))
    , nonce_(nonce)
{
}

Ping::BitcoinFormat_60001::BitcoinFormat_60001() noexcept
    : nonce_()
{
    static_assert(8 == sizeof(BitcoinFormat_60001));
}

Ping::BitcoinFormat_60001::BitcoinFormat_60001(
    const bitcoin::Nonce nonce) noexcept
    : nonce_(nonce)
{
    static_assert(8 == sizeof(BitcoinFormat_60001));
}

auto Ping::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto bytes = sizeof(BitcoinFormat_60001);
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = BitcoinFormat_60001{nonce_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), bytes);
        std::advance(i, bytes);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
