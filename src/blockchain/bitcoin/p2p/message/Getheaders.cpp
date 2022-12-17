// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
auto BitcoinP2PGetheaders(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::internal::Getheaders*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Getheaders;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto version = blockchain::p2p::bitcoin::ProtocolVersionField{};
        deserialize_object(bytes, version, "version");
        const auto count = decode_compact_size(bytes, "block hash count");
        auto hashes = Vector<blockchain::block::Hash>{};
        constexpr auto size = blockchain::block::Hash::payload_size_;

        for (auto i = 0_uz; i < count; ++i) {
            hashes.emplace_back(extract_prefix(bytes, size, "block hash"));
        }

        auto stop =
            blockchain::block::Hash{extract_prefix(bytes, size, "stop hash")};
        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            version.value(),
            std::move(hashes),
            std::move(stop));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
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
