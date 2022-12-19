// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/p2p/message/Getblocktxn.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocktxn(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    ReadView bytes) -> blockchain::p2p::bitcoin::message::Getblocktxn*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::Getblocktxn;
        using blockchain::block::Hash;

        if (false == pHeader.operator bool()) {

            throw std::runtime_error{"invalid header"};
        }

        auto block_hash = Hash{};
        deserialize_object(bytes, block_hash, "block hash");
        const auto indices = decode_compact_size(bytes, "index count");
        auto txn_indices = UnallocatedVector<std::size_t>{};

        for (auto i = 0_uz; i < indices; ++i) {
            txn_indices.emplace_back(decode_compact_size(bytes, "index "));
        }

        check_finished(bytes);

        return new ReturnType(api, std::move(pHeader), block_hash, txn_indices);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

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

auto Getblocktxn::payload(Writer&& out) const noexcept -> bool
{
    try {
        auto bytes = block_hash_.size();
        auto data = UnallocatedVector<ByteArray>{};
        data.reserve(1u + txn_indices_.size());
        const auto count = CompactSize(txn_indices_.size()).Encode();
        bytes += count.size();

        for (const auto& index : txn_indices_) {
            const auto cs = CompactSize(index).Encode();
            bytes += cs.size();
        }

        auto output = out.Reserve(bytes);

        if (false == output.IsValid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        auto* i = output.as<std::byte>();
        std::memcpy(i, block_hash_.data(), block_hash_.size());
        std::advance(i, block_hash_.size());

        for (const auto& cs : data) {
            std::memcpy(i, cs.data(), cs.size());
            std::advance(i, cs.size());
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
    , block_hash_(block_hash)
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
    , block_hash_(block_hash)
    , txn_indices_(txn_indices)
{
    verify_checksum();
}

}  // namespace opentxs::blockchain::p2p::bitcoin::message
