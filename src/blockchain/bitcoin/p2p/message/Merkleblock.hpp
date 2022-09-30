// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Merkleblock final : public implementation::Message
{
public:
    struct Raw {
        BlockHeaderField block_header_;
        TxnCountField txn_count_;

        Raw(const Data& block_header, const TxnCount txn_count) noexcept;
        Raw() noexcept;
    };

    auto getBlockHeader() const noexcept -> ByteArray { return block_header_; }
    auto getTxnCount() const noexcept -> TxnCount { return txn_count_; }
    auto getHashes() const noexcept -> const UnallocatedVector<ByteArray>&
    {
        return hashes_;
    }
    auto getFlags() const noexcept -> const UnallocatedVector<std::byte>&
    {
        return flags_;
    }

    Merkleblock(
        const api::Session& api,
        const blockchain::Type network,
        const Data& block_header,
        const TxnCount txn_count,
        const UnallocatedVector<ByteArray>& hashes,
        const UnallocatedVector<std::byte>& flags) noexcept;
    Merkleblock(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const Data& block_header,
        const TxnCount txn_count,
        const UnallocatedVector<ByteArray>& hashes,
        const UnallocatedVector<std::byte>& flags) noexcept(false);
    Merkleblock(const Merkleblock&) = delete;
    Merkleblock(Merkleblock&&) = delete;
    auto operator=(const Merkleblock&) -> Merkleblock& = delete;
    auto operator=(Merkleblock&&) -> Merkleblock& = delete;

    ~Merkleblock() final = default;

private:
    const ByteArray block_header_;
    const TxnCount txn_count_{};
    const UnallocatedVector<ByteArray> hashes_;
    const UnallocatedVector<std::byte> flags_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
