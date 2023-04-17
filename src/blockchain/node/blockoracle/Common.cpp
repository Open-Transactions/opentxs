// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/blockoracle/Types.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <stdexcept>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::node::blockoracle
{
static constexpr auto file_position_marker_ = std::byte{0};
static constexpr auto inline_block_marker_ = std::byte{1};

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
    auto in = frame.Bytes();

    if (0_uz == in.size()) { return MissingBlock{}; }

    try {
        auto marker = std::byte{};
        check_at_least(in, sizeof(marker), "marker");
        deserialize_object(in, marker, "marker");

        switch (marker) {
            case file_position_marker_: {
                auto ptr = std::uintptr_t{};
                auto bytes = std::size_t{};
                deserialize_object(in, ptr, "offset");
                deserialize_object(in, bytes, "length");

                return PersistentBlock{reinterpret_cast<char*>(ptr), bytes};
            }
            case inline_block_marker_: {

                return std::make_shared<ByteArray>(in);
            }
            default: {

                throw std::runtime_error{"unrecognized marker byte"};
            }
        }
    } catch (const std::exception& e) {
        LogAbort()(e.what()).Abort();
    }
}

auto reader(const BlockLocation& in, alloc::Default monotonic) noexcept
    -> ReadView
{
    struct Visitor {
        alloc::Default alloc_;

        auto operator()(const MissingBlock&) noexcept -> ReadView { return {}; }
        auto operator()(const PersistentBlock& block) noexcept -> ReadView
        {
            return block;
        }
        auto operator()(const CachedBlock& block) noexcept -> ReadView
        {
            OT_ASSERT(block);

            return block->Bytes();
        }
    };

    return std::visit(Visitor{monotonic}, in);
}

auto serialize(const BlockLocation& bytes, Writer&& out) noexcept -> bool
{
    struct Visitor {
        Writer&& out_;

        auto operator()(const MissingBlock&) noexcept -> bool { return false; }
        auto operator()(const PersistentBlock& block) noexcept -> bool
        {
            if (false == valid(block)) { return false; }

            const auto ptr = reinterpret_cast<std::uintptr_t>(block.data());
            const auto bytes = block.size();
            const auto size =
                sizeof(file_position_marker_) + sizeof(ptr) + sizeof(bytes);

            try {
                auto buf = reserve(std::move(out_), size, "file position");
                serialize_object(file_position_marker_, buf, "marker");
                serialize_object(ptr, buf, "offset");
                serialize_object(bytes, buf, "length");

                return true;
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return false;
            }
        }
        auto operator()(const CachedBlock& block) noexcept -> bool
        {
            const auto size = sizeof(inline_block_marker_) + block->size();

            try {
                auto buf = reserve(std::move(out_), size, "inline block");
                serialize_object(inline_block_marker_, buf, "marker");
                copy(block->Bytes(), buf, "block");

                return true;
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return false;
            }
        }
    };

    return std::visit(Visitor{std::move(out)}, bytes);
}
}  // namespace opentxs::blockchain::node::blockoracle
