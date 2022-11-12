// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"  // IWYU pragma: associated

#include <array>
#include <tuple>
#include <utility>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
struct CompactSize::Imp {
    using ThresholdDefinition = std::pair<std::size_t, std::byte>;
    using ThresholdData = std::array<ThresholdDefinition, 3>;

    static constexpr auto threshold_ = ThresholdData{
        {{252u, std::byte{0xfd}},
         {65535u, std::byte{0xfe}},
         {4294967295u, std::byte{0xff}}}};

    std::uint64_t data_;

    template <typename SizeType>
    auto convert_to_raw(Writer&& output) const noexcept -> bool;
    template <typename SizeType>
    auto convert_from_raw(ReadView bytes) noexcept -> void;

    Imp() noexcept
        : data_()
    {
    }
    Imp(std::uint64_t value) noexcept
        : data_(value)
    {
        static_assert(sizeof(data_) >= sizeof(std::size_t));
    }
    Imp(const Imp&) = default;
    Imp(Imp&&) = delete;
    auto operator=(const Imp& rhs) -> Imp& = default;
    auto operator=(Imp&&) noexcept -> Imp& = delete;
};
}  // namespace opentxs::network::blockchain::bitcoin
