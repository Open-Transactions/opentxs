// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/p2p/message/Ping.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PPing(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Ping*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Ping;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto raw = ReturnType::BitcoinFormat_60001{};

        if (bytes.size() >= sizeof(raw)) {
            deserialize_object(bytes, raw, "nonce");
        }

        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), raw.nonce_.value());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
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

auto Ping::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto data = BitcoinFormat_60001{nonce_};

        return copy(reader(std::addressof(data), sizeof(data)), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
