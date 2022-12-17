// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Size.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
template auto convert_to_size<std::uint64_t, std::size_t>(
    std::uint64_t) noexcept(false) -> std::size_t;
template auto convert_to_size<std::size_t, std::uint32_t>(std::size_t) noexcept(
    false) -> std::uint32_t;

auto decode_compact_size(ReadView& in, const std::string_view msg) noexcept(
    false) -> std::size_t
{
    using network::blockchain::bitcoin::DecodeCompactSize;

    if (auto out = DecodeCompactSize(in); out) {

        return convert_to_size(*out);
    } else {
        const auto error = UnallocatedCString{"failed to decode "}.append(msg);

        throw std::runtime_error{error.c_str()};
    }
}
}  // namespace opentxs
