// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Inv.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/Inventory.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PInv(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::internal::Inv*
{
    namespace bitcoin = blockchain::p2p::bitcoin::message;
    using ReturnType = bitcoin::implementation::Inv;

    try {
        if (false == bool(pHeader)) {
            throw std::runtime_error{"Invalid header"};
        }

        auto expectedSize = sizeof(std::byte);

        if (expectedSize > size) {
            throw std::runtime_error{"Size below minimum for Inv 1"};
        }

        auto* it{static_cast<const std::byte*>(payload)};
        std::size_t count{0};
        const bool haveCount = network::blockchain::bitcoin::DecodeSize(
            it, expectedSize, size, count);

        if (false == haveCount) {
            throw std::runtime_error{"CompactSize incomplete"};
        }

        UnallocatedVector<blockchain::bitcoin::Inventory> items{};

        if (count > 0) {
            for (std::size_t i{0}; i < count; ++i) {
                expectedSize += ReturnType::value_type::EncodedSize;

                if (expectedSize > size) {
                    throw std::runtime_error{
                        UnallocatedCString{
                            "Inventory entries incomplete at entry index "} +
                        std::to_string(i)};
                }

                items.emplace_back(it, ReturnType::value_type::EncodedSize);
                it += ReturnType::value_type::EncodedSize;
            }
        }

        return new ReturnType(api, std::move(pHeader), std::move(items));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BitcoinP2PInv(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Inv*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Inv;

    return new ReturnType(api, network, std::move(payload));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Inv::Inv(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<value_type>&& payload) noexcept
    : Message(api, network, bitcoin::Command::inv)
    , payload_(std::move(payload))
{
    init_hash();
}

Inv::Inv(
    const api::Session& api,
    std::unique_ptr<Header> header,
    UnallocatedVector<value_type>&& payload) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(payload))
{
}

auto Inv::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto fixed = value_type::size();
        const auto count = payload_.size();
        const auto cs = CompactSize(count).Encode();
        const auto bytes = cs.size() + (count * fixed);
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& inv : payload_) {
            if (false == inv.Serialize(preallocated(fixed, i))) {
                throw std::runtime_error{"failed to serialize inv"};
            }

            std::advance(i, fixed);
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
