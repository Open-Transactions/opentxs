// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Merkleblock.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PMerkleblock(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Merkleblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Merkleblock;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto raw_item = ReturnType::Raw{};

    auto expectedSize = sizeof(raw_item);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Merkleblock 1")
            .Flush();

        return nullptr;
    }
    const auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    std::memcpy(static_cast<void*>(&raw_item), it, sizeof(raw_item));
    it += sizeof(raw_item);

    const auto block_header =
        ByteArray{raw_item.block_header_.data(), raw_item.block_header_.size()};
    const bitcoin::TxnCount txn_count = raw_item.txn_count_.value();
    // --------------------------------------------------------
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Merkleblock 1")
            .Flush();

        return nullptr;
    }

    std::size_t hashCount{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, hashCount);

    if (!decodedSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    UnallocatedVector<ByteArray> hashes;

    if (hashCount > 0) {
        for (std::size_t ii = 0; ii < hashCount; ii++) {
            expectedSize += sizeof(bitcoin::BlockHeaderHashField);

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Hash entries incomplete at entry index ")(ii)
                    .Flush();

                return nullptr;
            }

            hashes.push_back(
                ByteArray{it, sizeof(bitcoin::BlockHeaderHashField)});
            it += sizeof(bitcoin::BlockHeaderHashField);
        }
    }
    // --------------------------------------------------------
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Merkleblock 1")
            .Flush();

        return nullptr;
    }

    std::size_t flagByteCount{0};
    const bool decodedFlagSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, flagByteCount);

    if (!decodedFlagSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    UnallocatedVector<std::byte> flags;

    if (flagByteCount > 0) {
        expectedSize += flagByteCount;

        if (expectedSize > size) {
            LogError()("opentxs::factory::")(__func__)(
                ": Flag field incomplete")
                .Flush();

            return nullptr;
        }

        UnallocatedVector<std::byte> temp_flags(flagByteCount);
        std::memcpy(temp_flags.data(), it, flagByteCount);
        std::advance(it, flagByteCount);
        flags = temp_flags;
    }
    // --------------------------------------------------------
    try {
        return new ReturnType(
            api, std::move(pHeader), block_header, txn_count, hashes, flags);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PMerkleblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block_header,
    const std::uint32_t txn_count,
    const UnallocatedVector<ByteArray>& hashes,
    const UnallocatedVector<std::byte>& flags)
    -> blockchain::p2p::bitcoin::message::Merkleblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Merkleblock;

    return new ReturnType(api, network, block_header, txn_count, hashes, flags);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Merkleblock::Merkleblock(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block_header,
    const TxnCount txn_count,
    const UnallocatedVector<ByteArray>& hashes,
    const UnallocatedVector<std::byte>& flags) noexcept
    : Message(api, network, bitcoin::Command::merkleblock)
    , block_header_(block_header)
    , txn_count_(txn_count)
    , hashes_(hashes)
    , flags_(flags)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Merkleblock::Merkleblock(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& block_header,
    const TxnCount txn_count,
    const UnallocatedVector<ByteArray>& hashes,
    const UnallocatedVector<std::byte>& flags) noexcept(false)
    : Message(api, std::move(header))
    , block_header_(block_header)
    , txn_count_(txn_count)
    , hashes_(hashes)
    , flags_(flags)
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

auto Merkleblock::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        static constexpr auto fixed = sizeof(Raw);
        const auto hashes = hashes_.size();
        const auto flags = flags_.size();
        const auto cs1 = CompactSize(hashes).Encode();
        const auto cs2 = CompactSize(flags).Encode();
        const auto bytes = fixed + cs1.size() + (hashes * standard_hash_size_) +
                           cs2.size() + flags;
        auto output = out(bytes);

        if (false == output.valid(bytes)) {
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
