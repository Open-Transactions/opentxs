// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocks(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocks*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocks;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    ReturnType::Raw raw_item;

    auto expectedSize = sizeof(raw_item.version_);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocks 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(
        static_cast<void*>(&raw_item.version_), it, sizeof(raw_item.version_));
    it += sizeof(raw_item.version_);
    bitcoin::ProtocolVersionUnsigned version = raw_item.version_.value();
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocks 1")
            .Flush();

        return nullptr;
    }

    std::pmr::vector<OTData> header_hashes;

    std::size_t hashCount{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, hashCount);

    if (!decodedSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    if (hashCount > 0) {
        for (std::size_t ii = 0; ii < hashCount; ii++) {
            expectedSize += sizeof(bitcoin::BlockHeaderHashField);

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Header hash entries incomplete at entry index ")(ii)
                    .Flush();

                return nullptr;
            }

            header_hashes.push_back(
                Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField)));
            it += sizeof(bitcoin::BlockHeaderHashField);
        }
    }
    // --------------------------------------------------------
    expectedSize += sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Stop hash entry missing or incomplete")
            .Flush();

        return nullptr;
    }

    auto stop_hash = Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField));
    it += sizeof(bitcoin::BlockHeaderHashField);

    try {
        return new ReturnType(
            api,
            std::move(pHeader),
            version,
            header_hashes,
            std::move(stop_hash));
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PGetblocks(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint32_t version,
    const std::pmr::vector<OTData>& header_hashes,
    const Data& stop_hash) -> blockchain::p2p::bitcoin::message::Getblocks*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocks;

    return new ReturnType(api, network, version, header_hashes, stop_hash);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Getblocks::Getblocks(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersionUnsigned version,
    const std::pmr::vector<OTData>& header_hashes,
    const Data& stop_hash) noexcept
    : Message(api, network, bitcoin::Command::getblocks)
    , version_(version)
    , header_hashes_(header_hashes)
    , stop_hash_(Data::Factory(stop_hash))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Getblocks::Getblocks(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    const std::pmr::vector<OTData>& header_hashes,
    const Data& stop_hash) noexcept(false)
    : Message(api, std::move(header))
    , version_(version)
    , header_hashes_(header_hashes)
    , stop_hash_(Data::Factory(stop_hash))
{
    verify_checksum();
}

auto Getblocks::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto hashBytes = sizeof(BlockHeaderHashField);
        const auto data = Raw{version_, header_hashes_, stop_hash_};
        const auto hashes = data.header_hashes_.size();
        const auto cs = CompactSize(hashes).Encode();
        const auto bytes = sizeof(data.version_) + cs.size() +
                           (hashes * hashBytes) + hashBytes;
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, &data.version_, sizeof(data.version_));
        std::advance(i, sizeof(data.version_));
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& hash : data.header_hashes_) {
            std::memcpy(i, hash.data(), hashBytes);
            std::advance(i, hashBytes);
        }

        std::memcpy(i, data.stop_hash_.data(), hashBytes);
        std::advance(i, hashBytes);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message
