// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
class Writer;
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

    auto getBlockHeader() const noexcept -> const ByteArray&
    {
        return block_header_;
    }
    auto getTxnCount() const noexcept -> TxnCount { return txn_count_; }
    auto getHashes() const noexcept -> const Vector<block::Hash>&
    {
        return hashes_;
    }
    auto getFlags() const noexcept -> const ByteArray& { return flags_; }

    Merkleblock(
        const api::Session& api,
        const blockchain::Type network,
        const TxnCount txn_count,
        ReadView block_header,
        Vector<block::Hash>&& hashes,
        ByteArray&& flags) noexcept;
    Merkleblock(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const TxnCount txn_count,
        ReadView block_header,
        Vector<block::Hash>&& hashes,
        ByteArray&& flags) noexcept(false);
    Merkleblock(const Merkleblock&) = delete;
    Merkleblock(Merkleblock&&) = delete;
    auto operator=(const Merkleblock&) -> Merkleblock& = delete;
    auto operator=(Merkleblock&&) -> Merkleblock& = delete;

    ~Merkleblock() final = default;

private:
    const TxnCount txn_count_;
    const ByteArray block_header_;
    const Vector<block::Hash> hashes_;
    const ByteArray flags_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
