// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/Secret.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "core/SecretPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Allocator.hpp"

namespace opentxs
{
Secret::Secret(SecretPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(imp_);

    imp_->parent_ = this;
}

Secret::Secret(const Secret& rhs) noexcept
    : Secret(construct<SecretPrivate>(
          {alloc::Secure::get()},
          rhs.data(),
          rhs.size(),
          rhs.imp_->Mode()))
{
}

Secret::Secret(Secret&& rhs) noexcept
    : Secret(std::exchange(rhs.imp_, nullptr))
{
}

auto Secret::asHex() const -> UnallocatedCString { return imp_->asHex(); }

auto Secret::asHex(alloc::Default alloc) const -> CString
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

auto Secret::at(const std::size_t position) -> std::byte&
{
    return imp_->at(position);
}

auto Secret::at(const std::size_t position) const -> const std::byte&
{
    return imp_->at(position);
}

auto Secret::begin() -> iterator { return imp_->begin(); }

auto Secret::begin() const -> const_iterator { return cbegin(); }

auto Secret::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto Secret::cbegin() const -> const_iterator { return imp_->cbegin(); }

auto Secret::cend() const -> const_iterator { return imp_->cend(); }

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

auto Secret::data() -> void* { return imp_->data(); }

auto Secret::data() const -> const void* { return imp_->data(); }

auto Secret::DecodeHex(const ReadView hex) -> bool
{
    return imp_->DecodeHex(hex);
}

auto Secret::empty() const -> bool { return imp_->empty(); }

auto Secret::end() -> iterator { return imp_->end(); }

auto Secret::end() const -> const_iterator { return cend(); }

auto Secret::Extract(
    const std::size_t amount,
    Data& output,
    const std::size_t pos) const -> bool
{
    return imp_->Extract(amount, output, pos);
}

auto Secret::Extract(std::uint16_t& output, const std::size_t pos) const -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint32_t& output, const std::size_t pos) const -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint64_t& output, const std::size_t pos) const -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::Extract(std::uint8_t& output, const std::size_t pos) const -> bool
{
    return imp_->Extract(output, pos);
}

auto Secret::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Secret::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Secret::operator=(const Secret& rhs) noexcept -> Secret&
{
    auto* old{imp_};
    {
        auto alloc = alloc::PMR<SecretPrivate>{alloc::Secure::get()};
        // TODO c++20
        imp_ = alloc.allocate(1_uz);

        OT_ASSERT(nullptr != imp_);

        alloc.construct(imp_, rhs.data(), rhs.size(), rhs.imp_->Mode());
    }
    {
        auto alloc = alloc::PMR<SecretPrivate>{old->get_allocator()};
        // TODO c++20
        alloc.destroy(old);
        alloc.deallocate(old, 1_uz);
    }

    return *this;
}

auto Secret::operator=(Secret&& rhs) noexcept -> Secret&
{
    swap(rhs);

    return *this;
}

auto Secret::IsNull() const -> bool { return imp_->IsNull(); }

auto Secret::Randomize(const std::size_t size) -> bool
{
    return imp_->Randomize(size);
}

auto Secret::resize(const std::size_t size) -> bool
{
    return imp_->resize(size);
}

auto Secret::SetSize(const std::size_t size) -> bool
{
    return imp_->SetSize(size);
}

auto Secret::size() const -> std::size_t { return imp_->size(); }

auto Secret::swap(Secret& rhs) noexcept -> void
{
    pmr_swap(*this, rhs, imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);
}

auto Secret::WriteInto() noexcept -> Writer { return imp_->WriteInto(); }

auto Secret::WriteInto(Mode mode) noexcept -> Writer
{
    return imp_->WriteInto(mode);
}

auto Secret::zeroMemory() -> void { imp_->zeroMemory(); }

Secret::~Secret() { pmr_delete(imp_); }
}  // namespace opentxs
