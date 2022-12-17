// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: associated

#include "opentxs/core/ByteArray.hpp"

namespace opentxs::blockchain::bitcoin::block::internal
{
auto Header::Blank() noexcept -> Header&
{
    static auto blank = Header{};

    return blank;
}
auto Header::Encode() const noexcept -> ByteArray { return {}; }

auto Header::MerkleRoot() const noexcept -> const blockchain::block::Hash&
{
    return Hash();
}

auto Header::Nonce() const noexcept -> std::uint32_t { return {}; }

auto Header::Timestamp() const noexcept -> Time { return {}; }

auto Header::Version() const noexcept -> std::uint32_t { return {}; }

auto Header::nBits() const noexcept -> std::uint32_t { return {}; }
}  // namespace opentxs::blockchain::bitcoin::block::internal
