// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Sendcmpct.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

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
    const auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    auto raw_item = ReturnType::Raw{};
    std::memcpy(static_cast<void*>(&raw_item), it, sizeof(raw_item));
    std::advance(it, sizeof(raw_item));
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

auto Sendcmpct::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto data = Sendcmpct::Raw{announce_, version_};

        return copy(reader(std::addressof(data), sizeof(data)), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
