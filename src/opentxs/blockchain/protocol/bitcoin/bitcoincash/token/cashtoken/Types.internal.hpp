// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/cashtoken/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network

namespace protobuf
{
class BlockchainTransactionOutput;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
{
struct Value {
    using marker_type = std::uint8_t;

    static constexpr auto nft_mask_ = marker_type{0b00100000};
    static constexpr auto capability_mask_ = marker_type{0b00001111};
    static constexpr auto commitment_mask_ = marker_type{0b01000000};
    static constexpr auto amount_mask_ = marker_type{0b00010000};

    FixedByteArray<32> category_{};
    bool nft_{};
    Capability capability_{};
    std::optional<ByteArray> commitment_{};
    std::optional<Amount> amount_{};

    auto Bytes() const noexcept -> std::size_t;
    auto Serialize(Writer&& out) const noexcept(false) -> void;
    auto Serialize(protobuf::BlockchainTransactionOutput& out) const noexcept
        -> void;
    auto View() const noexcept -> cashtoken::View;

private:
    auto bytes(
        network::blockchain::bitcoin::CompactSize* commitment,
        network::blockchain::bitcoin::CompactSize* amount) const noexcept
        -> std::size_t;
    auto has_amount() const noexcept -> bool;
    auto has_commitment() const noexcept -> bool;
};

auto deserialize(const protobuf::BlockchainTransactionOutput& in) noexcept(
    false) -> std::optional<Value>;
auto deserialize(ReadView& bytes, std::optional<Value>& out) noexcept(false)
    -> void;
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
