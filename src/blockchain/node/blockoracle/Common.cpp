// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/blockoracle/Types.hpp"  // IWYU pragma: associated

#include <cstring>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::node::blockoracle
{
auto is_valid(const BlockLocation& in) noexcept -> bool
{
    struct Visitor {
        auto operator()(const MissingBlock&) noexcept -> bool { return false; }
        auto operator()(const PersistentBlock&) noexcept -> bool
        {
            return true;
        }
        auto operator()(const CachedBlock&) noexcept -> bool { return true; }
    };

    return std::visit(Visitor{}, in);
}

auto parse_block_location(const network::zeromq::Frame& frame) noexcept
    -> BlockLocation
{
    constexpr auto target = sizeof(SerializedReadView);

    if (0_uz == frame.size()) {

        return MissingBlock{};
    } else if (frame.size() == target) {
        auto out = SerializedReadView{};
        auto* p = reinterpret_cast<std::byte*>(std::addressof(out));
        std::memcpy(p, frame.data(), target);

        return out;
    } else {

        return std::make_shared<ByteArray>(frame.Bytes());
    }
}

auto reader(const BlockLocation& in) noexcept -> ReadView
{
    struct Visitor {
        auto operator()(const MissingBlock&) noexcept -> ReadView { return {}; }
        auto operator()(const PersistentBlock& bytes) noexcept -> ReadView
        {
            return bytes;
        }
        auto operator()(const CachedBlock& block) noexcept -> ReadView
        {
            OT_ASSERT(block);

            return block->Bytes();
        }
    };

    return std::visit(Visitor{}, in);
}

auto serialize(const BlockLocation& bytes, Writer&& out) noexcept -> bool
{
    struct Visitor {
        Writer&& out_;

        auto operator()(const MissingBlock&) noexcept -> bool { return false; }
        auto operator()(const PersistentBlock& bytes) noexcept -> bool
        {
            const auto s = SerializedReadView{bytes};

            return copy(s.Bytes(), std::move(out_));
        }
        auto operator()(const CachedBlock& block) noexcept -> bool
        {
            return copy(block->Bytes(), std::move(out_));
        }
    };

    return std::visit(Visitor{std::move(out)}, bytes);
}
}  // namespace opentxs::blockchain::node::blockoracle
