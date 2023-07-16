// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/WriteBuffer.hpp"

#pragma once

#include <cstddef>
#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class WriteBuffer;
class WriterPrivate;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT Writer final : virtual public Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final;
    [[nodiscard]] auto Reserve(std::size_t) noexcept -> WriteBuffer;
    [[nodiscard]] auto Truncate(std::size_t) noexcept -> bool;

    Writer(
        std::function<WriteBuffer(std::size_t)> reserve,
        std::function<bool(std::size_t)> truncate = {},
        allocator_type alloc = {}) noexcept;
    OPENTXS_NO_EXPORT Writer(WriterPrivate* imp) noexcept;
    Writer() noexcept;
    Writer(const Writer&) = delete;
    Writer(Writer&& rhs) noexcept;
    Writer(Writer&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Writer&) -> Writer& = delete;
    auto operator=(Writer&& rhs) -> Writer& = delete;

    ~Writer() final;

private:
    WriterPrivate* imp_;
};
}  // namespace opentxs
