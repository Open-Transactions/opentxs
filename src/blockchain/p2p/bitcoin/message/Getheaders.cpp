// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getheaders.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

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

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&version), it, sizeof(version));
    it += sizeof(version);
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    std::size_t count{0};
    const bool haveCount =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == haveCount) {
        LogError()(__func__)(": Invalid CompactSize").Flush();

        return nullptr;
    }

    UnallocatedVector<blockchain::block::pHash> hashes{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize +=
                sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Header hash entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            hashes.emplace_back(Data::Factory(
                it, sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField)));
            it += sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);
        }
    }

    expectedSize += sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Stop hash entry missing or incomplete")
            .Flush();

        return nullptr;
    }

    auto stop = Data::Factory(
        it, sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField));

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
    UnallocatedVector<blockchain::block::pHash>&& history,
    blockchain::block::pHash&& stop)
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
    UnallocatedVector<block::pHash>&& hashes,
    block::pHash&& stop) noexcept
    : Message(api, network, bitcoin::Command::getheaders)
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(std::move(stop))
{
    init_hash();
}

Getheaders::Getheaders(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    UnallocatedVector<block::pHash>&& hashes,
    block::pHash&& stop) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(std::move(stop))
{
}

auto Getheaders::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto fixed =
            sizeof(ProtocolVersionField) + standard_hash_size_;
        const auto hashes = payload_.size();
        const auto cs = CompactSize(hashes).Encode();
        const auto bytes = fixed + cs.size() + (hashes * standard_hash_size_);
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
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
            std::memcpy(i, hash->data(), standard_hash_size_);
            std::advance(i, standard_hash_size_);
        }

        const auto& stop = [this]() -> const auto&
        {
            if (standard_hash_size_ == stop_->size()) {

                return stop_.get();
            } else {
                static const auto blank = block::BlankHash();

                return blank.get();
            }
        }
        ();

        std::memcpy(i, stop.data(), stop.size());
        std::advance(i, stop.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
