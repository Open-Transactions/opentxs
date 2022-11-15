// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Filteradd.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PFilteradd(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Filteradd*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Filteradd;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto data = extract_prefix(bytes, bytes.size(), "");
        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), ByteArray{data});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PFilteradd(
    const api::Session& api,
    const blockchain::Type network,
    const Data& element)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filteradd;

    return new ReturnType(api, network, element);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Filteradd::Filteradd(
    const api::Session& api,
    const blockchain::Type network,
    const Data& element) noexcept
    : Message(api, network, bitcoin::Command::filteradd)
    , element_(element)
{
    init_hash();
}

Filteradd::Filteradd(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& element) noexcept
    : Message(api, std::move(header))
    , element_(element)
{
}

auto Filteradd::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto element = element_.size();
        const auto cs = CompactSize(element).Encode();
        const auto bytes = cs.size() + element;
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());
        std::memcpy(i, element_.data(), element);
        std::advance(i, element);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
