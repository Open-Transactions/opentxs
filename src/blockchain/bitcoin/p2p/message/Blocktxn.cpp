// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Blocktxn.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PBlocktxn(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Blocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Blocktxn;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    return new ReturnType(
        api,
        std::move(pHeader),
        ByteArray{static_cast<const std::byte*>(payload), size});
}

auto BitcoinP2PBlocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& payload)
    -> blockchain::p2p::bitcoin::message::internal::Blocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Blocktxn;

    return new ReturnType(api, network, payload);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Blocktxn::Blocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& payload) noexcept
    : Message(api, network, bitcoin::Command::blocktxn)
    , payload_(payload)
{
    init_hash();
}

Blocktxn::Blocktxn(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& payload) noexcept
    : Message(api, std::move(header))
    , payload_(payload)
{
}

auto Blocktxn::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (false == copy(payload_.Bytes(), out)) {
            throw std::runtime_error{"failed to serialize payload"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
