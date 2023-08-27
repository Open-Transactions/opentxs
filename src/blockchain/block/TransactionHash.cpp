// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "core/FixedByteArray.tpp"

namespace opentxs::blockchain::block
{
TransactionHash::TransactionHash() noexcept
    : FixedByteArray()
{
    static_assert(payload_size_ == 32u);
}

TransactionHash::TransactionHash(const ReadView bytes) noexcept(false)
    : FixedByteArray(bytes)
{
}

TransactionHash::TransactionHash(const HexType&, const ReadView bytes) noexcept(
    false)
    : FixedByteArray()
{
    if (false == DecodeHex(bytes)) { throw std::runtime_error{"invalid hash"}; }
}

TransactionHash::TransactionHash(const TransactionHash& rhs) noexcept = default;

auto TransactionHash::operator=(const TransactionHash& rhs) noexcept
    -> TransactionHash& = default;

TransactionHash::~TransactionHash() = default;
}  // namespace opentxs::blockchain::block
