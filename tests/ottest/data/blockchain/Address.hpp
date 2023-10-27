// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <span>
#include <string_view>
#include <utility>

namespace ottest
{
using Base58Address = opentxs::Map<
    std::string_view,
    std::pair<
        opentxs::blockchain::crypto::AddressStyle,
        opentxs::Set<opentxs::blockchain::Type>>>;
using SegwitAddress = opentxs::Map<
    std::string_view,
    std::pair<opentxs::blockchain::Type, std::string_view>>;

OPENTXS_EXPORT auto Base58Addresses() noexcept -> const Base58Address&;
OPENTXS_EXPORT auto SegwitInvalid() noexcept
    -> std::span<const std::string_view>;
OPENTXS_EXPORT auto SegwitP2WPKH() noexcept -> const SegwitAddress&;
OPENTXS_EXPORT auto SegwitP2WSH() noexcept -> const SegwitAddress&;
}  // namespace ottest
