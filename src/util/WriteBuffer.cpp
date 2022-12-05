// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/WriteBuffer.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

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

        return size() == target;
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
auto WriteBuffer::RemovePrefix(std::size_t bytes) noexcept -> bool
{
    const auto original{bytes};
    const auto size = this->size();
    bytes = std::min(bytes, size);
    buf_ = {std::next(as<std::byte>(), bytes), size - bytes};

    return original == bytes;
}

auto WriteBuffer::Write(std::size_t bytes) noexcept -> Writer
{
    const auto size = this->size();
    bytes = std::min(bytes, size);
    auto out = preallocated(bytes, data());
    buf_ = {std::next(as<std::byte>(), bytes), size - bytes};

    return out;
}
}  // namespace opentxs
