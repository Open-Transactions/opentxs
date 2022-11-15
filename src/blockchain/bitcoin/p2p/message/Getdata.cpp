// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Getdata.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/Inventory.hpp"
#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PGetdata(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Getdata*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin::message;
        using ReturnType = bitcoin::implementation::Getdata;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto count = decode_compact_size(bytes, "inv count");
        auto items = UnallocatedVector<blockchain::bitcoin::Inventory>{};

        for (auto i = 0_uz; i < count; ++i) {
            static const auto size = ReturnType::value_type::EncodedSize;
            items.emplace_back(extract_prefix(bytes, size, "inv"));
        }

        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), std::move(items));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PGetdata(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Getdata*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getdata;

    return new ReturnType(api, network, std::move(payload));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getdata::Getdata(
    const api::Session& api,
    const blockchain::Type network,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload) noexcept
    : Message(api, network, bitcoin::Command::getdata)
    , payload_(std::move(payload))
{
    init_hash();
}

Getdata::Getdata(
    const api::Session& api,
    std::unique_ptr<Header> header,
    UnallocatedVector<blockchain::bitcoin::Inventory>&& payload) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(payload))
{
}

auto Getdata::payload(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto fixed = value_type::size();
        const auto count = payload_.size();
        const auto cs = CompactSize(count).Encode();
        const auto bytes = cs.size() + (count * fixed);
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
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
