// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/p2p/message/Getblocks.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocks(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Getblocks*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::Getblocks;
        using blockchain::block::Hash;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto raw = ReturnType::Raw{};
        deserialize_object(bytes, raw.version_, "version");
        const auto count = decode_compact_size(bytes, "block hash count");
        auto hashes = Vector<Hash>{};
        constexpr auto hash = sizeof(bitcoin::BlockHeaderHashField);

        for (auto i = 0_uz; i < count; ++i) {
            hashes.emplace_back(extract_prefix(bytes, hash, "hash"));
        }

        auto stop = extract_prefix(bytes, hash, "stop hash");
        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            raw.version_.value(),
            std::move(hashes),
            Hash{stop});
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PGetblocks(
    const api::Session& api,
    const blockchain::Type network,
    const std::uint32_t version,
    Vector<blockchain::block::Hash>&& header_hashes,
    blockchain::block::Hash&& stop_hash)
    -> blockchain::p2p::bitcoin::message::Getblocks*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocks;

    return new ReturnType(
        api, network, version, std::move(header_hashes), std::move(stop_hash));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Getblocks::Getblocks(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersionUnsigned version,
    Vector<block::Hash>&& header_hashes,
    block::Hash&& stop_hash) noexcept
    : Message(api, network, bitcoin::Command::getblocks)
    , version_(version)
    , header_hashes_(std::move(header_hashes))
    , stop_hash_(std::move(stop_hash))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Getblocks::Getblocks(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    Vector<block::Hash>&& header_hashes,
    block::Hash&& stop_hash) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , header_hashes_(header_hashes)
    , stop_hash_(stop_hash)
{
    verify_checksum();
}

auto Getblocks::payload(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto hashBytes = sizeof(BlockHeaderHashField);
        const auto data = Raw{version_, header_hashes_, stop_hash_};
        const auto hashes = data.header_hashes_.size();
        const auto cs = CompactSize(hashes).Encode();
        const auto bytes = sizeof(data.version_) + cs.size() +
                           (hashes * hashBytes) + hashBytes;
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
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
}  // namespace opentxs::blockchain::p2p::bitcoin::message
