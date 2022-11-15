// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Cfcheckpt.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PCfcheckpt(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Cfcheckpt;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto& header = *pHeader;
        auto raw = ReturnType::BitcoinFormat{};
        deserialize_object(bytes, raw, "prefix");
        const auto count = decode_compact_size(bytes, "cfheader count");
        auto cfheaders = Vector<blockchain::cfilter::Header>{};

        for (auto i = 0_uz; i < count; ++i) {
            constexpr auto size = blockchain::cfilter::Header::payload_size_;
            cfheaders.emplace_back(extract_prefix(bytes, size, "cfheader"));
        }

        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            raw.Type(header.Network()),
            raw.Hash(),
            std::move(cfheaders));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinP2PCfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& stop,
    Vector<blockchain::cfilter::Header>&& headers)
    -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfcheckpt;

    return new ReturnType(api, network, type, stop, std::move(headers));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Cfcheckpt::Cfcheckpt(
    const api::Session& api,
    const blockchain::Type network,
    const cfilter::Type type,
    const block::Hash& stop,
    Vector<cfilter::Header>&& headers) noexcept
    : Message(api, network, bitcoin::Command::cfcheckpt)
    , type_(type)
    , stop_(stop)
    , payload_(std::move(headers))
{
    init_hash();
}

Cfcheckpt::Cfcheckpt(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const cfilter::Type type,
    const block::Hash& stop,
    Vector<cfilter::Header>&& headers) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , stop_(stop)
    , payload_(std::move(headers))
{
}

auto Cfcheckpt::payload(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto fixed = sizeof(BitcoinFormat);
        const auto hashes = payload_.size();
        const auto cs = CompactSize(hashes).Encode();
        const auto bytes = fixed + cs.size() + (hashes * standard_hash_size_);
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = BitcoinFormat{header().Network(), type_, stop_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), fixed);
        std::advance(i, fixed);
        std::memcpy(i, cs.data(), cs.size());
        std::advance(i, cs.size());

        for (const auto& hash : payload_) {
            std::memcpy(i, hash.data(), standard_hash_size_);
            std::advance(i, standard_hash_size_);
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
