// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/cfilter/Header.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "core/FixedByteArray.tpp"

namespace opentxs::blockchain::cfilter
{
Header::Header() noexcept
    : FixedByteArray()
{
    static_assert(payload_size_ == 32u);
}

Header::Header(const ReadView bytes) noexcept(false)
    : FixedByteArray(bytes)
{
}

Header::Header(const HexType&, const ReadView bytes) noexcept(false)
    : FixedByteArray()
{
    if (false == DecodeHex(bytes)) {
        throw std::runtime_error{"invalid cfheader"};
    }
}

Header::Header(const Header& rhs) noexcept = default;

auto Header::operator=(const Header& rhs) noexcept -> Header& = default;

Header::~Header() = default;
}  // namespace opentxs::blockchain::cfilter
