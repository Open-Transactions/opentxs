// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocktxn(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocktxn;

    if (false == bool(pHeader)) {
        LogError()("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocktxn 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    OTData block_hash =
        Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField));
    it += sizeof(bitcoin::BlockHeaderHashField);
    // --------------------------------------------------------
    // Next load up the transaction count (CompactSize)

    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogError()("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocktxn 1")
            .Flush();

        return nullptr;
    }

    std::size_t indicesCount{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, indicesCount);

    if (!decodedSize) {
        LogError()(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    UnallocatedVector<std::size_t> txn_indices;

    if (indicesCount > 0) {
        for (std::size_t ii = 0; ii < indicesCount; ii++) {
            expectedSize += sizeof(std::byte);

            if (expectedSize > size) {
                LogError()("opentxs::factory::")(__func__)(
                    ": Txn index entries incomplete at entry index ")(ii)
                    .Flush();

                return nullptr;
            }

            std::size_t txnIndex{0};
            const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
                it, expectedSize, size, txnIndex);

            if (!decodedSize) {
                LogError()(__func__)(": CompactSize incomplete").Flush();

                return nullptr;
            }

            txn_indices.push_back(txnIndex);
        }
    }
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), block_hash, txn_indices);
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PGetblocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block_hash,
    const UnallocatedVector<std::size_t>& txn_indices)
    -> blockchain::p2p::bitcoin::message::Getblocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocktxn;

    return new ReturnType(api, network, block_hash, txn_indices);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{

auto Getblocktxn::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        auto bytes = block_hash_->size();
        auto data = UnallocatedVector<OTData>{};
        data.reserve(1u + txn_indices_.size());
        const auto& count = data.emplace_back(
            Data::Factory(CompactSize(txn_indices_.size()).Encode()));
        bytes += count->size();

        for (const auto& index : txn_indices_) {
            const auto& cs =
                data.emplace_back(Data::Factory(CompactSize(index).Encode()));
            bytes += cs->size();
        }

        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, block_hash_->data(), block_hash_->size());
        std::advance(i, block_hash_->size());

        for (const auto& cs : data) {
            std::memcpy(i, cs->data(), cs->size());
            std::advance(i, cs->size());
        }
        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

// We have all the data members to create the message from scratch (for sending)
Getblocktxn::Getblocktxn(
    const api::Session& api,
    const blockchain::Type network,
    const Data& block_hash,
    const UnallocatedVector<std::size_t>& txn_indices) noexcept
    : Message(api, network, bitcoin::Command::getblocktxn)
    , block_hash_(Data::Factory(block_hash))
    , txn_indices_(txn_indices)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Getblocktxn::Getblocktxn(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const Data& block_hash,
    const UnallocatedVector<std::size_t>& txn_indices) noexcept(false)
    : Message(api, std::move(header))
    , block_hash_(Data::Factory(block_hash))
    , txn_indices_(txn_indices)
{
    verify_checksum();
}

}  // namespace opentxs::blockchain::p2p::bitcoin::message
