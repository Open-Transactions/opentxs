// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <cstdint>
#include <span>
#include <string_view>

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

struct OPENTXS_EXPORT BlockHeaderTestSequence {
    const std::string_view value_;

    auto operator<=>(const BlockHeaderTestSequence& rhs) const noexcept
        -> std::strong_ordering;
    constexpr auto operator==(const BlockHeaderTestSequence& rhs) const noexcept
        -> bool
    {
        return value_ == rhs.value_;
    }
};
struct OPENTXS_EXPORT BlockHeaderTag {
    const std::string_view value_;

    constexpr auto empty() const noexcept -> bool { return value_.empty(); }
    auto operator<=>(const BlockHeaderTag& rhs) const noexcept
        -> std::strong_ordering;
    constexpr auto operator==(const BlockHeaderTag& rhs) const noexcept -> bool
    {
        return value_ == rhs.value_;
    }
};
struct OPENTXS_EXPORT BlockHeaderCreateParams {
    const BlockHeaderTag parent_;
    const BlockHeaderTag child_;
};
struct OPENTXS_EXPORT BlockPositionTag {
    const ot::blockchain::block::Height height_;
    const BlockHeaderTag block_;
};
struct OPENTXS_EXPORT BlockHeaderStatus {
    const BlockHeaderTag parent_;
    const ot::blockchain::block::Height height_;
    const std::uint32_t status_;
    const std::uint32_t parent_status_;
};
struct OPENTXS_EXPORT BlockchainSnapshot {
    const ot::Map<BlockHeaderTag, BlockHeaderStatus> status_;
    const ot::Vector<BlockHeaderTag> best_chain_;
    const ot::Set<BlockHeaderTag> orphans_;
};
struct OPENTXS_EXPORT BlockHeaderSequence {
    const ot::Vector<BlockHeaderCreateParams> create_;
    const ot::Vector<BlockchainSnapshot> state_;
};

constexpr auto block_1_ = BlockHeaderTag{"block 01_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_2_ = BlockHeaderTag{"block 02_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_3_ = BlockHeaderTag{"block 03_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_4_ = BlockHeaderTag{"block 04_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_5_ = BlockHeaderTag{"block 05_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_6_ = BlockHeaderTag{"block 06_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_7_ = BlockHeaderTag{"block 07_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_8_ = BlockHeaderTag{"block 08_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_9_ = BlockHeaderTag{"block 09_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_10_ = BlockHeaderTag{"block 10_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_11_ = BlockHeaderTag{"block 11_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_12_ = BlockHeaderTag{"block 12_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_13_ = BlockHeaderTag{"block 13_XXXXXXXXXXXXXXXXXXXXXXX"sv};
constexpr auto block_14_ = BlockHeaderTag{"block 14_XXXXXXXXXXXXXXXXXXXXXXX"sv};

// clang-format off
//  1   2   3   4   5   6   7   8   9   10
constexpr auto block_sequence_1_ = BlockHeaderTestSequence{"basic_sequence"sv};

//  1   2   3   7   8
//          4
//          5   6
constexpr auto block_sequence_2_ = BlockHeaderTestSequence{"basic_reorg"sv};

//  1   2   3   4   5   6
//              7
//             (8)  9   10
constexpr auto block_sequence_3_ = BlockHeaderTestSequence{"checkpoint_prevents_update"sv};

//  1   2   3  (4)  10
//              5
//              6   7   8   9
constexpr auto block_sequence_4_ = BlockHeaderTestSequence{"checkpoint_prevents_reorg"sv};

//                      7   8   9
//  1   2   3   12  4   5   6   13  14
//              10  11
constexpr auto block_sequence_5_ = BlockHeaderTestSequence{"receive_headers_out_of_order"sv};

//  1   2   3   4   5  (6)  7   8   9   10
constexpr auto block_sequence_6_ = BlockHeaderTestSequence{"add_checkpoint_already_in_best_chain"sv};

//  1   2   3   7   8
//         (4)
//          5   6
constexpr auto block_sequence_7_ = BlockHeaderTestSequence{"reorg_to_checkpoint"sv};

//          8   9
//     (6)  7
//  1   2   3   4   5
constexpr auto block_sequence_8_ = BlockHeaderTestSequence{"reorg_to_checkpoint_descendent"sv};

//  1   2   3   7   8
//          4
//          5   6
//
//     (9)
constexpr auto block_sequence_9_ = BlockHeaderTestSequence{"add_checkpoint_disconnected"sv};

//  delete_checkpoint
//          8   9
//     (6)  7
//  1   2   3   4   5
constexpr auto block_sequence_10_ = BlockHeaderTestSequence{"delete_checkpoint"sv};
// clang-format on

auto OPENTXS_EXPORT BitcoinHeaders() noexcept
    -> std::span<const std::string_view>;
auto OPENTXS_EXPORT BitcoinHeaderTestSequences() noexcept
    -> const ot::Map<BlockHeaderTestSequence, BlockHeaderSequence>&;
}  // namespace ottest
