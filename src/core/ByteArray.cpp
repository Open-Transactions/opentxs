// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/ByteArray.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <stdexcept>
#include <utility>

#include "core/ByteArrayPrivate.hpp"
#include "internal/core/Armored.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto swap(ByteArray& lhs, ByteArray& rhs) noexcept -> void
{
    return lhs.swap(rhs);
}
}  // namespace opentxs

namespace opentxs
{
ByteArray::ByteArray(ByteArrayPrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

ByteArray::ByteArray(allocator_type alloc) noexcept
    : ByteArray(pmr::default_construct<ByteArrayPrivate>(alloc))
{
}

ByteArray::ByteArray(std::uint8_t in, allocator_type alloc) noexcept
    : ByteArray(sizeof(in), std::addressof(in), alloc)
{
}

ByteArray::ByteArray(std::uint16_t in, allocator_type alloc) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint16_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));

        return pmr::construct<ByteArrayPrivate>(
            alloc, std::addressof(buf), sizeof(buf));
    }())
{
}

ByteArray::ByteArray(std::uint32_t in, allocator_type alloc) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint32_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));

        return pmr::construct<ByteArrayPrivate>(
            alloc, std::addressof(buf), sizeof(buf));
    }())
{
}

ByteArray::ByteArray(std::uint64_t in, allocator_type alloc) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint64_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));

        return pmr::construct<ByteArrayPrivate>(
            alloc, std::addressof(buf), sizeof(buf));
    }())
{
}

ByteArray::ByteArray(const ReadView bytes, allocator_type alloc) noexcept
    : ByteArray(alloc)
{
    Assign(bytes);
}

ByteArray::ByteArray(
    const HexType&,
    const ReadView bytes,
    allocator_type alloc) noexcept(false)
    : ByteArray(alloc)
{
    if (false == DecodeHex(bytes)) {
        throw std::invalid_argument{"unable to decode input as hex"};
    }
}

ByteArray::ByteArray(const Armored& rhs, allocator_type alloc) noexcept
    : ByteArray(alloc)
{
    if (rhs.Exists()) { rhs.GetData(*this); }
}

ByteArray::ByteArray(const Data& rhs, allocator_type alloc) noexcept
    : ByteArray(alloc)
{
    Assign(rhs);
}

ByteArray::ByteArray(const ByteArray& rhs, allocator_type alloc) noexcept
    : ByteArray(rhs.imp_->clone(alloc))
{
}

ByteArray::ByteArray(ByteArray&& rhs) noexcept
    : ByteArray(std::exchange(rhs.imp_, nullptr))
{
}

ByteArray::ByteArray(ByteArray&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

ByteArray::ByteArray(
    std::size_t size,
    const void* data,
    allocator_type alloc) noexcept
    : ByteArray(pmr::construct<ByteArrayPrivate>(alloc, data, size))
{
}

auto ByteArray::operator=(const ByteArray& rhs) noexcept -> ByteArray&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto ByteArray::operator=(ByteArray&& rhs) noexcept -> ByteArray&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto ByteArray::asHex() const noexcept -> UnallocatedCString
{
    return imp_->asHex();
}

auto ByteArray::asHex(alloc::Default alloc) const noexcept -> CString
{
    return imp_->asHex(alloc);
}

auto ByteArray::Assign(const Data& source) noexcept -> bool
{
    return imp_->Assign(source);
}

auto ByteArray::Assign(const ReadView source) noexcept -> bool
{
    return imp_->Assign(source);
}

auto ByteArray::Assign(const void* data, const std::size_t size) noexcept
    -> bool
{
    return imp_->Assign(data, size);
}

auto ByteArray::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto ByteArray::clear() noexcept -> void { imp_->clear(); }

auto ByteArray::Concatenate(const ReadView in) noexcept -> bool
{
    return imp_->Concatenate(in);
}

auto ByteArray::Concatenate(const void* data, const std::size_t size) noexcept
    -> bool
{
    return imp_->Concatenate(data, size);
}

auto ByteArray::data() noexcept -> void* { return imp_->data(); }

auto ByteArray::data() const noexcept -> const void* { return imp_->data(); }

auto ByteArray::DecodeHex(const ReadView hex) noexcept -> bool
{
    return imp_->DecodeHex(hex);
}

auto ByteArray::empty() const noexcept -> bool { return imp_->empty(); }

auto ByteArray::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const noexcept -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto ByteArray::Extract(std::uint16_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint32_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint64_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint8_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::get() const noexcept -> std::span<const std::byte>
{
    return imp_->get();
}

auto ByteArray::get() noexcept -> std::span<std::byte> { return imp_->get(); }

auto ByteArray::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto ByteArray::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto ByteArray::IsNull() const noexcept -> bool { return imp_->IsNull(); }

auto ByteArray::operator+=(const Data& rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::operator+=(const ReadView rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::operator+=(const std::uint16_t rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::operator+=(const std::uint32_t rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::operator+=(const std::uint64_t rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::operator+=(const std::uint8_t rhs) noexcept -> ByteArray&
{
    imp_->operator+=(rhs);

    return *this;
}

auto ByteArray::pop_front() noexcept -> void { imp_->pop_front(); }

auto ByteArray::Randomize(const std::size_t size) noexcept -> bool
{
    return imp_->Randomize(size);
}

auto ByteArray::resize(const std::size_t size) noexcept -> bool
{
    return imp_->resize(size);
}

auto ByteArray::size() const noexcept -> std::size_t { return imp_->size(); }

auto ByteArray::swap(ByteArray& rhs) noexcept -> void
{
    pmr::swap(imp_, rhs.imp_);
}

auto ByteArray::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

ByteArray::~ByteArray() { pmr::destroy(imp_); }
}  // namespace opentxs
