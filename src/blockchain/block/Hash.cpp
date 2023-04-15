// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "core/FixedByteArray.tpp"

namespace opentxs::blockchain::block
{
Hash::Hash() noexcept
    : FixedByteArray()
{
    static_assert(payload_size_ == 32u);
}

Hash::Hash(const ReadView bytes) noexcept(false)
    : FixedByteArray(bytes)
{
}

Hash::Hash(const HexType&, const ReadView bytes) noexcept(false)
    : FixedByteArray()
{
    if (false == DecodeHex(bytes)) { throw std::runtime_error{"invalid hash"}; }
}

Hash::Hash(const Hash& rhs) noexcept
    : FixedByteArray(rhs)
{
}

auto Hash::operator=(const Hash& rhs) noexcept -> Hash& = default;

Hash::~Hash() = default;
}  // namespace opentxs::blockchain::block
