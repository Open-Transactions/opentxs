// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::file
{
constexpr auto index_bytes_ = 2_uz * sizeof(std::uint64_t);
}  // namespace opentxs::storage::file

namespace opentxs::storage::file
{
class Index
{
public:
    auto empty() const noexcept -> bool;
    auto ItemSize() const noexcept -> std::size_t;
    auto MemoryPosition() const noexcept -> std::size_t;
    auto Serialize() const noexcept -> FixedByteArray<index_bytes_>;
    auto Serialize(Writer&& destination) const noexcept -> bool;

    auto Deserialize(ReadView bytes) noexcept(false) -> void;
    auto SetItemSize(std::size_t size) noexcept -> void;
    auto SetMemoryPosition(std::size_t position) noexcept -> void;

    Index(std::size_t position, std::size_t size) noexcept;
    Index() noexcept;

private:
    std::size_t position_;
    std::size_t size_;
};
}  // namespace opentxs::storage::file
