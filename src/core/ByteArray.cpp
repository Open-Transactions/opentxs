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
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
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
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

ByteArray::ByteArray(allocator_type a) noexcept
    : ByteArray([&] {
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{a};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp);

        return imp;
    }())
{
}

ByteArray::ByteArray(std::uint8_t in, allocator_type alloc) noexcept
    : ByteArray(sizeof(in), std::addressof(in), alloc)
{
}

ByteArray::ByteArray(std::uint16_t in, allocator_type a) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint16_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{a};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp, std::addressof(buf), sizeof(buf));

        return imp;
    }())
{
}

ByteArray::ByteArray(std::uint32_t in, allocator_type a) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint32_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{a};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp, std::addressof(buf), sizeof(buf));

        return imp;
    }())
{
}

ByteArray::ByteArray(std::uint64_t in, allocator_type a) noexcept
    : ByteArray([&] {
        const auto buf = boost::endian::big_uint64_buf_t(in);
        static_assert(sizeof(in) == sizeof(buf));
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{a};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp, std::addressof(buf), sizeof(buf));

        return imp;
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
    : ByteArray(alloc)
{
    Assign(rhs);
}

ByteArray::ByteArray(ByteArray&& rhs) noexcept
    : ByteArray(rhs.imp_)
{
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
    rhs.imp_ = nullptr;
}

ByteArray::ByteArray(ByteArray&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    OT_ASSERT(nullptr != rhs.imp_);

    if (rhs.get_allocator() == alloc) {
        imp_ = rhs.imp_;
        rhs.imp_ = nullptr;
    } else {
        // TODO c++20
        auto pmr = alloc::PMR<ByteArrayPrivate>{alloc};
        imp_ = pmr.allocate(1);
        pmr.construct(imp_, rhs.data(), rhs.size());
    }

    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

ByteArray::ByteArray(
    std::size_t size,
    const void* data,
    allocator_type a) noexcept
    : ByteArray([&] {
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{a};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp, data, size);

        return imp;
    }())
{
}

auto ByteArray::operator=(const ByteArray& rhs) noexcept -> ByteArray&
{
    OT_ASSERT(nullptr != rhs.imp_);

    auto alloc = [&]() -> alloc::PMR<ByteArrayPrivate> {
        if (nullptr != imp_) {

            return get_allocator();
        } else {

            return rhs.get_allocator();
        }
    }();
    auto* old = imp_;
    imp_ = [&] {
        // TODO c++20
        auto* imp = alloc.allocate(1);
        alloc.construct(imp, rhs.data(), rhs.size());

        return imp;
    }();

    OT_ASSERT(nullptr != imp_);

    if (nullptr != old) {
        // TODO c++20
        auto pmr = alloc::PMR<ByteArrayPrivate>{old->get_allocator()};
        pmr.destroy(old);
        pmr.deallocate(old, 1);
    }

    return *this;
}

auto ByteArray::operator=(ByteArray&& rhs) noexcept -> ByteArray&
{

    if ((nullptr == imp_) || (get_allocator() == rhs.get_allocator())) {
        swap(rhs);

        return *this;
    } else {

        return operator=(const_cast<const ByteArray&>(rhs));
    }
}

auto ByteArray::asHex() const -> UnallocatedCString { return imp_->asHex(); }

auto ByteArray::asHex(alloc::Default alloc) const -> CString
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

auto ByteArray::at(const std::size_t position) -> std::byte&
{
    return imp_->at(position);
}

auto ByteArray::at(const std::size_t position) const -> const std::byte&
{
    return imp_->at(position);
}

auto ByteArray::begin() -> iterator { return imp_->begin(); }

auto ByteArray::begin() const -> const_iterator { return cbegin(); }

auto ByteArray::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto ByteArray::cbegin() const -> const_iterator { return imp_->cbegin(); }

auto ByteArray::cend() const -> const_iterator { return imp_->cend(); }

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

auto ByteArray::data() -> void* { return imp_->data(); }

auto ByteArray::data() const -> const void* { return imp_->data(); }

auto ByteArray::DecodeHex(const ReadView hex) -> bool
{
    return imp_->DecodeHex(hex);
}

auto ByteArray::empty() const -> bool { return imp_->empty(); }

auto ByteArray::end() -> iterator { return imp_->end(); }

auto ByteArray::end() const -> const_iterator { return cend(); }

auto ByteArray::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto ByteArray::Extract(std::uint16_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint32_t& output, const std::size_t pos) const
    -> bool  // TODO c++20
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint64_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::Extract(std::uint8_t& output, const std::size_t pos) const
    -> bool
{
    return imp_->Extract(output, pos);
}

auto ByteArray::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto ByteArray::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto ByteArray::IsNull() const -> bool { return imp_->IsNull(); }

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

auto ByteArray::Randomize(const std::size_t size) -> bool
{
    return imp_->Randomize(size);
}

auto ByteArray::resize(const std::size_t size) -> bool
{
    return imp_->resize(size);
}

auto ByteArray::SetSize(const std::size_t size) -> bool
{
    return imp_->SetSize(size);
}

auto ByteArray::size() const -> std::size_t { return imp_->size(); }

auto ByteArray::swap(ByteArray& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);

    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;

    if (nullptr != rhs.imp_) { rhs.imp_->parent_ = std::addressof(rhs); }
}

auto ByteArray::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

auto ByteArray::zeroMemory() -> void { imp_->zeroMemory(); }

ByteArray::~ByteArray()
{
    if (nullptr != imp_) {
        // TODO c++20
        auto alloc = alloc::PMR<ByteArrayPrivate>{get_allocator()};
        alloc.destroy(imp_);
        alloc.deallocate(imp_, 1);
        imp_ = nullptr;
    }
}
}  // namespace opentxs
