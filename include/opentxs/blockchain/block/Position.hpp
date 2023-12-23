// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <string_view>
// IWYU pragma: no_include <typeindex>

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Position> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::block::Position& data)
        const noexcept -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::blockchain::block::Position> {
    auto operator()(
        const opentxs::blockchain::block::Position& lhs,
        const opentxs::blockchain::block::Position& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::blockchain::block
{
OPENTXS_EXPORT auto operator==(const Position&, const Position&) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Position&, const Position&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Position& lhs, Position& rhs) noexcept -> void;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Position
{
public:
    Height height_;
    Hash hash_;

    auto IsReplacedBy(const Position& rhs) const noexcept -> bool;
    auto NotReplacedBy(const Position& rhs) const noexcept -> bool;
    auto print() const noexcept -> UnallocatedCString;
    auto print(alloc::Default alloc) const noexcept -> CString;

    auto swap(Position& rhs) noexcept -> void;

    Position() noexcept;
    Position(const Height& height, const Hash& hash) noexcept;
    Position(const Height& height, Hash&& hash) noexcept;
    Position(const Height& height, ReadView hash) noexcept;
    Position(Height&& height, const Hash& hash) noexcept;
    Position(Height&& height, Hash&& hash) noexcept;
    Position(Height&& height, ReadView hash) noexcept;
    Position(const std::pair<Height, Hash>& data) noexcept;
    Position(std::pair<Height, Hash>&& data) noexcept;
    Position(const Position& rhs) noexcept;
    Position(Position&& rhs) noexcept;
    auto operator=(const Position& rhs) noexcept -> Position&;
    auto operator=(Position&& rhs) noexcept -> Position&;
};
}  // namespace opentxs::blockchain::block
