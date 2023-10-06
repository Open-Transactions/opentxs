// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Index.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::storage::file
{
Index::Index(std::size_t position, std::size_t size) noexcept
    : position_(position)
    , size_(size)
{
    static_assert(sizeof(std::size_t) <= sizeof(std::uint64_t));
}

Index::Index() noexcept
    : Index(0_uz, 0_uz)
{
}

auto Index::Deserialize(ReadView bytes) noexcept(false) -> void
{
    if (false == valid(bytes)) {

        throw std::runtime_error{"invalid byte range"};
    }

    auto buf = boost::endian::little_uint64_buf_t{};

    if (bytes.size() < index_bytes_) {
        throw std::runtime_error{"wrong size for byte range"};
    }

    const auto* i = reinterpret_cast<const std::byte*>(bytes.data());
    std::memcpy(static_cast<void*>(std::addressof(buf)), i, sizeof(buf));
    position_ = convert_to_size(buf.value());
    std::advance(i, sizeof(buf));
    std::memcpy(static_cast<void*>(std::addressof(buf)), i, sizeof(buf));
    size_ = convert_to_size(buf.value());
}

auto Index::empty() const noexcept -> bool { return 0_uz == size_; }

auto Index::ItemSize() const noexcept -> std::size_t { return size_; }

auto Index::MemoryPosition() const noexcept -> std::size_t { return position_; }

auto Index::Serialize() const noexcept -> FixedByteArray<index_bytes_>
{
    auto out = FixedByteArray<index_bytes_>{};
    Serialize(out.WriteInto());

    return out;
}

auto Index::Serialize(Writer&& destination) const noexcept -> bool
{
    auto dest = destination.Reserve(index_bytes_);

    if (false == dest.IsValid(index_bytes_)) {
        LogError()()("insufficient space for serialization").Flush();

        return false;
    }

    using Buffer = boost::endian::little_uint64_buf_t;
    static_assert(sizeof(position_) <= sizeof(Buffer));
    static_assert(sizeof(size_) <= sizeof(Buffer));
    auto* out = dest.as<std::byte>();
    auto buf = Buffer{position_};
    std::memcpy(out, std::addressof(buf), sizeof(buf));
    std::advance(out, sizeof(buf));
    buf = Buffer{size_};
    std::memcpy(out, std::addressof(buf), sizeof(buf));

    return true;
}

auto Index::SetItemSize(std::size_t size) noexcept -> void { size_ = size; }

auto Index::SetMemoryPosition(std::size_t position) noexcept -> void
{
    position_ = position;
}
}  // namespace opentxs::storage::file
