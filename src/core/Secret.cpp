// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "core/Secret.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>

#include "internal/core/Core.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::Secret>;

namespace opentxs::factory
{
auto Secret(const std::size_t bytes) noexcept
    -> std::unique_ptr<opentxs::Secret>
{
    using ReturnType = implementation::Secret;

    return std::make_unique<ReturnType>(bytes);
}
auto Secret(const ReadView bytes, const bool mode) noexcept
    -> std::unique_ptr<opentxs::Secret>
{
    using ReturnType = implementation::Secret;

    return std::make_unique<ReturnType>(
        bytes, static_cast<ReturnType::Mode>(mode));
}
}  // namespace opentxs::factory

namespace std
{
auto less<opentxs::Pimpl<opentxs::Secret>>::operator()(
    const opentxs::OTSecret& lhs,
    const opentxs::OTSecret& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs
{
auto operator==(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator==(rhs);
}
auto operator==(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator==(rhs);
}
auto operator!=(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator!=(rhs);
}
auto operator!=(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator!=(rhs);
}
auto operator<(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator<(rhs);
}
auto operator<(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator<(rhs);
}
auto operator>(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator>(rhs);
}
auto operator>(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator>(rhs);
}
auto operator<=(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator<=(rhs);
}
auto operator<=(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator<=(rhs);
}
auto operator>=(const OTSecret& lhs, const Secret& rhs) noexcept -> bool
{
    return lhs->operator>=(rhs);
}
auto operator>=(const OTSecret& lhs, const ReadView& rhs) noexcept -> bool
{
    return lhs->operator>=(rhs);
}
auto operator+=(OTSecret& lhs, const Secret& rhs) noexcept -> Secret&
{
    return lhs->operator+=(rhs);
}
auto operator+=(OTSecret& lhs, const ReadView rhs) noexcept -> Secret&
{
    return lhs->operator+=(rhs);
}
}  // namespace opentxs

namespace opentxs::implementation
{
constexpr auto effective_size(const std::size_t size, const Secret::Mode mode)
{
    return (Secret::Mode::Mem == mode) ? size : size + 1u;
}

const SecureAllocator<std::byte> Secret::allocator_{};

Secret::Secret(const std::size_t bytes) noexcept
    : mode_(Mode::Mem)
    , data_(bytes, std::byte{}, allocator_)
{
}
Secret::Secret(const ReadView bytes, const Mode mode) noexcept
    : mode_(mode)
    , data_(effective_size(bytes.size(), mode), std::byte{}, allocator_)
{
    std::memcpy(data_.data(), bytes.data(), bytes.size());
}
Secret::Secret(const Secret& rhs) noexcept
    : mode_(rhs.mode_)
    , data_(rhs.data_)
{
}
auto Secret::operator==(const ReadView rhs) const noexcept -> bool
{
    return 0 == spaceship(rhs);
}
auto Secret::operator!=(const ReadView rhs) const noexcept -> bool
{
    return 0 != spaceship(rhs);
}
auto Secret::operator<(const ReadView rhs) const noexcept -> bool
{
    return 0 > spaceship(rhs);
}
auto Secret::operator>(const ReadView rhs) const noexcept -> bool
{
    return 0 < spaceship(rhs);
}
auto Secret::operator<=(const ReadView& rhs) const noexcept -> bool
{
    return 0 >= spaceship(rhs);
}
auto Secret::operator>=(const ReadView& rhs) const noexcept -> bool
{
    return 0 <= spaceship(rhs);
}
auto Secret::operator+=(const opentxs::Secret& rhs) noexcept -> Secret&
{
    if (data() != rhs.data()) { Concatenate(rhs.data(), rhs.size()); }

    return *this;
}
auto Secret::operator+=(const ReadView rhs) noexcept -> Secret&
{
    if (data() != reinterpret_cast<const std::byte*>(rhs.data())) {
        Concatenate(rhs.data(), rhs.size());
    }

    return *this;
}
auto Secret::Assign(const void* data, const std::size_t& size) noexcept -> void
{
    mode_ = Mode::Mem;
    data_.clear();
    data_.reserve(size);
    data_.resize(size, {});
    std::memcpy(data_.data(), data, size);
}
auto Secret::AssignText(const ReadView rhs) noexcept -> void
{
    mode_ = Mode::Text;
    data_.clear();
    const auto targetSize = std::size_t{rhs.size() + 1u};
    data_.reserve(targetSize);
    data_.resize(targetSize, {});
    std::memcpy(data_.data(), rhs.data(), rhs.size());
}
auto Secret::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(data()), size()};
}
auto Secret::Concatenate(const void* in, const std::size_t bytes) noexcept
    -> void
{
    // Test for self-assignment
    const auto a = reinterpret_cast<std::uintptr_t>(data_.data());
    const auto b =
        reinterpret_cast<std::uintptr_t>(data_.data() + data_.size());
    const auto x = reinterpret_cast<std::uintptr_t>(in);
    const auto y = reinterpret_cast<std::uintptr_t>(
        static_cast<const std::byte*>(in) + bytes);

    if ((y < a) || (x > b)) {
        const auto targetSize = std::size_t{size() + bytes};
        Resize(targetSize);
        auto it = data();
        std::advance(it, size());
        std::memcpy(it, in, bytes);
    } else {
        OT_FAIL;
    }
}
auto Secret::Randomize(const std::size_t bytes) noexcept -> std::size_t
{
    if (size() != bytes) { Resize(bytes); }

    ::randombytes_buf(data(), size());

    return bytes;
}
auto Secret::Resize(const std::size_t bytes) noexcept -> std::size_t
{
    const auto target = mem() ? bytes : bytes + 1u;
    data_.resize(target);

    return size();
}

auto Secret::size() const noexcept -> std::size_t
{
    return data_.size() - (mem() ? 0u : 1u);
}

auto Secret::spaceship(const ReadView rhs) const noexcept -> int
{
    const auto bytes = size();
    const auto rBytes = rhs.size();

    if (bytes < rBytes) { return -1; }

    if (bytes > rBytes) { return 1; }

    return std::memcmp(data(), rhs.data(), bytes);
}
auto Secret::WriteInto(const std::optional<Mode> mode) noexcept
    -> AllocateOutput
{
    const auto binary{mode.has_value() ? (Mode::Mem == mode.value()) : mem()};

    return [binary, this](const auto size) {
        auto blank = space(size);

        if (binary) {
            Assign(blank.data(), blank.size());
        } else {
            AssignText(ReadView{
                reinterpret_cast<const char*>(blank.data()), blank.size()});
        }

        return WritableView{data(), this->size()};
    };
}
}  // namespace opentxs::implementation
