// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/Secret.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "core/SecretPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Allocator.hpp"

namespace opentxs
{
Secret::Secret(SecretPrivate* imp) noexcept
    : imp_(imp)
{
    assert_true(imp_);
}

Secret::Secret(const Secret& rhs) noexcept
    : Secret(rhs.imp_->clone({alloc::Secure::get()}))
{
}

Secret::Secret(Secret&& rhs) noexcept
    : Secret(std::exchange(rhs.imp_, nullptr))
{
}

auto Secret::asHex() const noexcept -> UnallocatedCString
{
    return imp_->asHex();
}

auto Secret::asHex(alloc::Default alloc) const noexcept -> CString
{
    return imp_->asHex(alloc);
}

auto Secret::Assign(const Data& source) noexcept -> bool
{
    return imp_->Assign(source.data(), source.size());
}

auto Secret::Assign(const ReadView source) noexcept -> bool
{
    return imp_->Assign(source.data(), source.size());
}

auto Secret::Assign(const void* data, const std::size_t size) noexcept -> bool
{
    return imp_->Assign(data, size);
}

auto Secret::AssignText(const ReadView source) noexcept -> bool
{
    return imp_->AssignText(source);
}

auto Secret::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto Secret::clear() noexcept -> void { imp_->clear(); }

auto Secret::Concatenate(const ReadView in) noexcept -> bool
{
    return imp_->Concatenate(in);
}

auto Secret::Concatenate(const void* data, const std::size_t size) noexcept
    -> bool
{
    return imp_->Concatenate(data, size);
}

auto Secret::data() noexcept -> void* { return imp_->data(); }

auto Secret::data() const noexcept -> const void* { return imp_->data(); }

auto Secret::DecodeHex(const ReadView hex) noexcept -> bool
{
    return imp_->DecodeHex(hex);
}

auto Secret::empty() const noexcept -> bool { return imp_->empty(); }

auto Secret::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const noexcept -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto Secret::Extract(std::uint16_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint32_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint64_t& output, const std::size_t pos)
    const noexcept -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint8_t& output, const std::size_t pos) const noexcept
    -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::get() const noexcept -> std::span<const std::byte>
{
    return imp_->get();
}

auto Secret::get() noexcept -> std::span<std::byte> { return imp_->get(); }

auto Secret::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Secret::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Secret::operator=(const Secret& rhs) noexcept -> Secret&
{
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto Secret::operator=(Secret&& rhs) noexcept -> Secret&
{
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Secret::IsNull() const noexcept -> bool { return imp_->IsNull(); }

auto Secret::Randomize(const std::size_t size) noexcept -> bool
{
    return imp_->Randomize(size);
}

auto Secret::resize(const std::size_t size) noexcept -> bool
{
    return imp_->resize(size);
}

auto Secret::size() const noexcept -> std::size_t { return imp_->size(); }

auto Secret::swap(Secret& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

auto Secret::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

auto Secret::WriteInto(Mode mode) noexcept -> Writer
{
    return imp_->WriteInto(mode);
}

Secret::~Secret() { pmr::destroy(imp_); }
}  // namespace opentxs
