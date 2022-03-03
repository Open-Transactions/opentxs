// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Feefilter.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
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
    auto* it{static_cast<const std::byte*>(payload)};

    FeeRateField raw_rate;
    std::memcpy(reinterpret_cast<std::byte*>(&raw_rate), it, sizeof(raw_rate));
    it += sizeof(raw_rate);

    const std::uint64_t fee_rate = raw_rate.value();

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

auto Feefilter::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto bytes = sizeof(FeeRateField);
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = FeeRateField{fee_rate_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, &data, bytes);
        std::advance(i, bytes);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
