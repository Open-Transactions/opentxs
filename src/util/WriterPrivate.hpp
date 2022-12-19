// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/WriteBuffer.hpp"

#pragma once

#include <cstddef>
#include <functional>
#include <optional>

#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class WriterPrivate final : public implementation::Allocated
{
public:
    [[nodiscard]] auto Reserve(std::size_t) noexcept -> WriteBuffer;
    [[nodiscard]] auto Truncate(std::size_t) noexcept -> bool;

    WriterPrivate(
        std::function<WriteBuffer(std::size_t)> reserve,
        std::function<bool(std::size_t)> truncate,
        allocator_type alloc) noexcept;
    WriterPrivate() = delete;
    WriterPrivate(const WriterPrivate&) = delete;
    WriterPrivate(WriterPrivate&& rhs) noexcept;
    auto operator=(const WriterPrivate&) -> WriterPrivate& = delete;
    auto operator=(WriterPrivate&& rhs) -> WriterPrivate& = delete;

    ~WriterPrivate() final;

private:
    const std::function<WriteBuffer(std::size_t)> reserve_;
    const std::function<bool(std::size_t)> truncate_;
    std::optional<std::size_t> size_;
};
}  // namespace opentxs
