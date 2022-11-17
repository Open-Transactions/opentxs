// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Merkleblock.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PMerkleblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Merkleblock*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::Merkleblock;
        using blockchain::block::Hash;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto raw = ReturnType::Raw{};
        deserialize_object(bytes, raw, "prefix");
        const auto txCount = raw.txn_count_.value();

        const auto hashCount = decode_compact_size(bytes, "hash count");
        auto hashes = Vector<Hash>{};

        for (auto i = 0_uz; i < hashCount; ++i) {
            constexpr auto size = sizeof(bitcoin::BlockHeaderHashField);
            hashes.push_back(extract_prefix(bytes, size, "hash"));
        }

        const auto flagByteCount =
            decode_compact_size(bytes, "flag byte count");
        auto flags = ByteArray{};
        deserialize(bytes, flags.WriteInto(), flagByteCount, "flag bytes");
        check_finished(bytes);

        return new ReturnType(
            api,
            std::move(pHeader),
            txCount,
            ReadView{
                reinterpret_cast<const char*>(raw.block_header_.data()),
                raw.block_header_.size()},
            std::move(hashes),
            std::move(flags));
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Merkleblock::Merkleblock(
    const api::Session& api,
    const blockchain::Type network,
    const TxnCount txn_count,
    ReadView block_header,
    Vector<block::Hash>&& hashes,
    ByteArray&& flags) noexcept
    : Message(api, network, bitcoin::Command::merkleblock)
    , txn_count_(txn_count)
    , block_header_(block_header)
    , hashes_(std::move(hashes))
    , flags_(std::move(flags))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Merkleblock::Merkleblock(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const TxnCount txn_count,
    ReadView block_header,
    Vector<block::Hash>&& hashes,
    ByteArray&& flags) noexcept(false)
    : Message(api, std::move(header))
    , txn_count_(txn_count)
    , block_header_(block_header)
    , hashes_(std::move(hashes))
    , flags_(std::move(flags))
{
    verify_checksum();
}

Merkleblock::Raw::Raw(
    const Data& block_header,
    const TxnCount txn_count) noexcept
    : block_header_()
    , txn_count_(txn_count)
{
    OT_ASSERT(sizeof(block_header_) == block_header.size());

    std::memcpy(block_header_.data(), block_header.data(), block_header.size());
}

Merkleblock::Raw::Raw() noexcept
    : block_header_()
    , txn_count_(0)
{
}

auto Merkleblock::payload(Writer&& out) const noexcept -> bool
{
    try {
        static constexpr auto fixed = sizeof(Raw);
        const auto hashes = hashes_.size();
        const auto flags = flags_.size();
        const auto cs1 = CompactSize(hashes).Encode();
        const auto cs2 = CompactSize(flags).Encode();
        const auto bytes = fixed + cs1.size() + (hashes * standard_hash_size_) +
                           cs2.size() + flags;
        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data = Raw{block_header_, txn_count_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data), fixed);
        std::advance(i, fixed);
        std::memcpy(i, cs1.data(), cs1.size());
        std::advance(i, cs1.size());

        for (const auto& hash : hashes_) {
            std::memcpy(i, hash.data(), standard_hash_size_);
            std::advance(i, standard_hash_size_);
        }

        std::memcpy(i, cs2.data(), cs2.size());
        std::advance(i, cs2.size());
        std::memcpy(i, flags_.data(), flags_.size());
        std::advance(i, flags_.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message
