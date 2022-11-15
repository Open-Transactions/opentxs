// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Getheaders.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getheaders;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    blockchain::p2p::bitcoin::ProtocolVersionField version{};
    auto expectedSize = sizeof(version);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (version)")
            .Flush();

        return nullptr;
    }

    const auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&version), it, sizeof(version));
    it += sizeof(version);
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto count = 0_uz;
    const bool haveCount =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == haveCount) {
        LogError()(__func__)(": Invalid CompactSize").Flush();

        return nullptr;
    }

    auto hashes = Vector<blockchain::block::Hash>{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += blockchain::block::Hash::payload_size_;

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Header hash entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            hashes.emplace_back(ReadView{
                reinterpret_cast<const char*>(it),
                blockchain::block::Hash::payload_size_});
            it += blockchain::block::Hash::payload_size_;
        }
    }

    expectedSize += blockchain::block::Hash::payload_size_;

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Stop hash entry missing or incomplete")
            .Flush();

        return nullptr;
    }

    auto stop = blockchain::block::Hash{ReadView{
        reinterpret_cast<const char*>(it),
        blockchain::block::Hash::payload_size_}};

    return new ReturnType(
        api,
        std::move(pHeader),
        version.value(),
        std::move(hashes),
        std::move(stop));
}

auto BitcoinP2PGetheaders(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersionUnsigned version,
    Vector<blockchain::block::Hash>&& history,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getheaders;

    return new ReturnType(
        api, network, version, std::move(history), std::move(stop));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getheaders::Getheaders(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersionUnsigned version,
    Vector<block::Hash>&& hashes,
    const block::Hash& stop) noexcept
    : Message(api, network, bitcoin::Command::getheaders)
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(stop)
{
    init_hash();
}

Getheaders::Getheaders(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    Vector<block::Hash>&& hashes,
    const block::Hash& stop) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(stop)
{
}

auto Getheaders::payload(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto fixed =
            sizeof(ProtocolVersionField) + standard_hash_size_;
        const auto hashes = payload_.size();
        const auto cs = CompactSize(hashes).Encode();
        const auto bytes = fixed + cs.size() + (hashes * standard_hash_size_);
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = ProtocolVersionField{version_};
        auto* i = output.as<std::byte>();
        std::memcpy(
            i, static_cast<const void*>(&data), sizeof(ProtocolVersionField));
        std::advance(i, sizeof(ProtocolVersionField));
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& hash : payload_) {
            std::memcpy(i, hash.data(), standard_hash_size_);
            std::advance(i, standard_hash_size_);
        }

        std::memcpy(i, stop_.data(), stop_.size());
        std::advance(i, stop_.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
