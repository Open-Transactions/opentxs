// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"  // IWYU pragma: associated

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
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto raw_cmpctblock(Data::Factory(payload, size));
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), raw_cmpctblock);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PCmpctblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
    -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    return new ReturnType(api, network, raw_cmpctblock);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Cmpctblock::Cmpctblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock) noexcept
    : Message(api, network, bitcoin::Command::cmpctblock)
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Cmpctblock::Cmpctblock(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& raw_cmpctblock) noexcept(false)
    : Message(api, std::move(header))
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    verify_checksum();
}

auto Cmpctblock::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        const auto bytes = raw_cmpctblock_->size();
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, raw_cmpctblock_->data(), bytes);
        std::advance(i, bytes);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
