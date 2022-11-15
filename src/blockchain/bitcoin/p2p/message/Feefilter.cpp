// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Feefilter.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
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

using FeeRateField = be::little_uint64_buf_t;

namespace opentxs::factory
{
auto BitcoinP2PFeefilter(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Feefilter*
{
    namespace be = boost::endian;
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Feefilter;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(FeeRateField);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Feefilter 1")
            .Flush();

        return nullptr;
    }
    const auto* it{static_cast<const std::byte*>(payload)};

    FeeRateField raw_rate;
    std::memcpy(reinterpret_cast<std::byte*>(&raw_rate), it, sizeof(raw_rate));
    std::advance(it, sizeof(raw_rate));
    const auto fee_rate = raw_rate.value();

    try {
        return new ReturnType(api, std::move(pHeader), fee_rate);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

auto BitcoinP2PFeefilter(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t fee_rate)
    -> blockchain::p2p::bitcoin::message::Feefilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Feefilter;

    return new ReturnType(api, network, fee_rate);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
Feefilter::Feefilter(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint64_t fee_rate) noexcept
    : Message(api, network, bitcoin::Command::feefilter)
    , fee_rate_(fee_rate)
{
    init_hash();
}

Feefilter::Feefilter(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const std::uint64_t fee_rate) noexcept(false)
    : Message(api, std::move(header))
    , fee_rate_(fee_rate)
{
    verify_checksum();
}

auto Feefilter::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto data = FeeRateField{fee_rate_};

        return copy(reader(std::addressof(data), sizeof(data)), std::move(out));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
