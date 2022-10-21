// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "opentxs/util/WriteBuffer.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/P0330.hpp"

namespace opentxs
{
WriteBuffer::WriteBuffer(std::span<std::byte>&& buf) noexcept
    : buf_(std::move(buf))
{
}

WriteBuffer::WriteBuffer() noexcept
    : WriteBuffer(std::span<std::byte>{})
{
}

auto WriteBuffer::IsValid(std::size_t target) const noexcept -> bool
{
    if (0_uz < target) {

        return buf_.size() == target;
    } else {

        return false;
    }
}

auto WriteBuffer::operator=(WriteBuffer&& rhs) noexcept -> WriteBuffer&
{
    using std::swap;
    swap(buf_, rhs.buf_);

    return *this;
}
}  // namespace opentxs
