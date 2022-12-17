// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/p2p/message/Cfilter.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BitcoinP2PCfilter(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Cfilter;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        const auto& header = *pHeader;
        auto raw = ReturnType::BitcoinFormat{};
        deserialize_object(bytes, raw, "prefix");
        const auto cfilterBytes = decode_compact_size(bytes, "cfheader size");
        check_exactly(bytes, cfilterBytes, "cfilter");
        using blockchain::internal::DecodeCfilterElementCount;
        const auto elementCount = DecodeCfilterElementCount(bytes);
        const auto filterType = raw.Type(header.Network());
        auto cfilter = extract_prefix(bytes, bytes.size(), "");
        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            filterType,
            raw.Hash(),
            elementCount,
            space(cfilter));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BitcoinP2PCfilter(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::cfilter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::GCS& filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    return new ReturnType(api, network, type, hash, filter.ElementCount(), [&] {
        auto out = Space{};
        filter.Compressed(writer(out));

        return out;
    }());
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Cfilter::Cfilter(
    const api::Session& api,
    const blockchain::Type network,
    const cfilter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    const Space& compressed) noexcept
    : Message(api, network, bitcoin::Command::cfilter)
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(compressed)
    , params_(blockchain::internal::GetFilterParams(type_))
{
    init_hash();
}

Cfilter::Cfilter(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const cfilter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    Space&& compressed) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(std::move(compressed))
    , params_(blockchain::internal::GetFilterParams(type_))
{
}

auto Cfilter::payload(Writer&& out) const noexcept -> bool
{
    try {
        const auto payload = [&] {
            auto filter = [&] {
                auto output = CompactSize(count_).Encode();
                output.Concatenate(reader(filter_));

                return output;
            }();
            auto output = CompactSize(filter.size()).Encode();
            output.Concatenate(filter.Bytes());

            return output;
        }();
        static constexpr auto fixed = sizeof(BitcoinFormat);
        const auto bytes = fixed + payload.size();
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = BitcoinFormat{header().Network(), type_, hash_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), fixed);
        std::advance(i, fixed);
        std::memcpy(i, payload.data(), payload.size());
        std::advance(i, payload.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
