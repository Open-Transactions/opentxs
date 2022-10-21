// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Block.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PBlock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Block*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Block;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    return new ReturnType(
        api,
        std::move(pHeader),
        ByteArray{static_cast<const std::byte*>(payload), size});
}

auto BitcoinP2PBlock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& raw_block)
    -> blockchain::p2p::bitcoin::message::internal::Block*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Block;

    return new ReturnType(api, network, raw_block);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Block::Block(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block) noexcept
    : Message(api, network, bitcoin::Command::block)
    , payload_(block)
{
    init_hash();
}

Block::Block(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& block) noexcept
    : Message(api, std::move(header))
    , payload_(block)
{
}

auto Block::payload(Writer&& out) const noexcept -> bool
{
    try {
        if (false == copy(payload_.Bytes(), std::move(out))) {
            throw std::runtime_error{"failed to serialize payload"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
