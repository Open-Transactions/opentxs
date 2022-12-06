// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "blockchain/bitcoin/p2p/message/Getcfcheckpt.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetcfcheckpt(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Getcfcheckpt;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto& header = *pHeader;
        auto raw = ReturnType::BitcoinFormat{};
        deserialize_object(bytes, raw, "prefix");
        check_finished(bytes);

        return new ReturnType(
            api, std::move(pHeader), raw.Type(header.Network()), raw.Hash());
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PGetcfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getcfcheckpt;

    return new ReturnType(api, network, type, stop);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getcfcheckpt::Getcfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const cfilter::Type type,
    const block::Hash& stop) noexcept
    : Message(api, network, bitcoin::Command::getcfcheckpt)
    , type_(type)
    , stop_(stop)
{
    init_hash();
}

Getcfcheckpt::Getcfcheckpt(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const cfilter::Type type,
    block::Hash&& stop) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , stop_(std::move(stop))
{
}

auto Getcfcheckpt::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto data = BitcoinFormat{header().Network(), type_, stop_};

        return copy(reader(std::addressof(data), sizeof(data)), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
