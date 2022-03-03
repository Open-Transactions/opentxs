// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PSendcmpct(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Sendcmpct*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Sendcmpct;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(ReturnType::Raw);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Sendcmpct 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    auto raw_item = ReturnType::Raw{};
    std::memcpy(static_cast<void*>(&raw_item), it, sizeof(raw_item));
    it += sizeof(raw_item);
    const bool announce = (0 == raw_item.announce_.value()) ? false : true;
    const std::uint64_t version = raw_item.version_.value();
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), announce, version);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PSendcmpct(
    const api::Session& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version)
    -> blockchain::p2p::bitcoin::message::Sendcmpct*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Sendcmpct;

    return new ReturnType(api, network, announce, version);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Sendcmpct::Sendcmpct(
    const api::Session& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version) noexcept
    : Message(api, network, bitcoin::Command::sendcmpct)
    , announce_(announce)
    , version_(version)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Sendcmpct::Sendcmpct(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bool announce,
    const std::uint64_t version) noexcept(false)
    : Message(api, std::move(header))
    , announce_(announce)
    , version_(version)
{
    verify_checksum();
}

Sendcmpct::Raw::Raw(bool announce, std::uint64_t version) noexcept
    : announce_(static_cast<std::int8_t>(announce ? 1 : 0))
    , version_(version)
{
}

Sendcmpct::Raw::Raw() noexcept
    : announce_(0)
    , version_(0)
{
}

auto Sendcmpct::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto bytes = sizeof(Sendcmpct::Raw);
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = Sendcmpct::Raw{announce_, version_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), bytes);
        std::advance(i, bytes);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
