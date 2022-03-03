// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/Header.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PHeader(
    const api::Session& api,
    const network::zeromq::Frame& bytes) -> blockchain::p2p::bitcoin::Header*
{
    using ReturnType = opentxs::blockchain::p2p::bitcoin::Header;
    const ReturnType::BitcoinFormat raw{bytes};

    return new ReturnType(
        api, raw.Network(), raw.Command(), raw.PayloadSize(), raw.Checksum());
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin
{
Header::Header(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::Command command,
    const std::size_t payload,
    const OTData checksum) noexcept
    : chain_(network)
    , command_(command)
    , payload_size_(payload)
    , checksum_(checksum)
{
}

Header::Header(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::Command command) noexcept
    : Header(api, network, command, 0, Data::Factory())
{
}

Header::BitcoinFormat::BitcoinFormat(
    const void* data,
    const std::size_t size) noexcept(false)
    : magic_()
    , command_()
    , length_()
    , checksum_()
{
    static_assert(header_size_ == sizeof(BitcoinFormat));

    if (sizeof(BitcoinFormat) != size) {
        throw std::invalid_argument("Incorrect input size");
    }

    std::memcpy(this, data, size);
}

Header::BitcoinFormat::BitcoinFormat(
    const blockchain::Type network,
    const bitcoin::Command command,
    const std::size_t payload,
    const OTData checksum) noexcept(false)
    : magic_(params::Data::Chains().at(network).p2p_magic_bits_)
    , command_(SerializeCommand(command))
    , length_(static_cast<std::uint32_t>(payload))
    , checksum_()
{
    static_assert(header_size_ == sizeof(BitcoinFormat));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= payload);

    if (sizeof(checksum_) != checksum->size()) {
        throw std::invalid_argument("Incorrect checksum size");
    }

    std::memcpy(checksum_.data(), checksum->data(), checksum->size());
}

Header::BitcoinFormat::BitcoinFormat(const Data& in) noexcept(false)
    : BitcoinFormat(in.data(), in.size())
{
}

Header::BitcoinFormat::BitcoinFormat(const zmq::Frame& in) noexcept(false)
    : BitcoinFormat(in.data(), in.size())
{
}

auto Header::BitcoinFormat::Checksum() const noexcept -> OTData
{
    return Data::Factory(checksum_.data(), checksum_.size());
}

auto Header::BitcoinFormat::Command() const noexcept -> bitcoin::Command
{
    return GetCommand(command_);
}

auto Header::BitcoinFormat::Network() const noexcept -> blockchain::Type
{
    static const auto build = []() -> auto
    {
        auto output = UnallocatedMap<std::uint32_t, blockchain::Type>{};

        for (const auto& [chain, data] : params::Data::Chains()) {
            if (0 != data.p2p_magic_bits_) {
                output.emplace(data.p2p_magic_bits_, chain);
            }
        }

        return output;
    };
    static const auto map{build()};

    try {

        return map.at(magic_.value());
    } catch (...) {

        return blockchain::Type::Unknown;
    }
}

auto Header::BitcoinFormat::PayloadSize() const noexcept -> std::size_t
{
    return length_.value();
}

auto Header::Serialize(const AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        auto bytes = out(header_size_);

        if (false == bytes.valid(header_size_)) {
            throw std::runtime_error{"failed to allocate write buffer"};
        }

        const auto raw =
            BitcoinFormat(chain_, command_, payload_size_, checksum_);
        std::memcpy(bytes.data(), static_cast<const void*>(&raw), header_size_);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Header::SetChecksum(const std::size_t payload, OTData&& checksum) noexcept
    -> void
{
    payload_size_ = payload;
    checksum_ = std::move(checksum);
}
}  // namespace opentxs::blockchain::p2p::bitcoin
