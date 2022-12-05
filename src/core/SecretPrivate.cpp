// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/SecretPrivate.hpp"  // IWYU pragma: associated
#include "internal/core/Core.hpp"  // IWYU pragma: associated

#include <cstring>
#include <functional>
#include <span>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Allocator.hpp"

namespace opentxs::factory
{
auto Secret(const std::size_t bytes) noexcept -> opentxs::Secret
{
    auto alloc = alloc::PMR<SecretPrivate>{alloc::Secure::get()};
    // TODO c++20
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(out);

    alloc.construct(out, bytes, Secret::Mode::Mem);

    return out;
}

auto Secret(const ReadView bytes, const bool mode) noexcept -> opentxs::Secret
{
    auto alloc = alloc::PMR<SecretPrivate>{alloc::Secure::get()};
    // TODO c++20
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(out);

    alloc.construct(
        out, bytes.data(), bytes.size(), static_cast<Secret::Mode>(mode));

    return out;
}
}  // namespace opentxs::factory

namespace opentxs
{
constexpr auto effective_size(const std::size_t size, const Secret::Mode mode)
{
    return (Secret::Mode::Mem == mode) ? size : size + 1_uz;
}

SecretPrivate::SecretPrivate(
    std::size_t size,
    Secret::Mode mode,
    allocator_type alloc) noexcept
    : ByteArrayPrivate(alloc)
    , mode_(mode)
{
    SetSize(effective_size(size, mode_));
}

SecretPrivate::SecretPrivate(
    const void* data,
    std::size_t size,
    Secret::Mode mode,
    allocator_type alloc) noexcept
    : ByteArrayPrivate(data, effective_size(size, mode), alloc)
    , mode_(mode)
{
}

auto SecretPrivate::Assign(const void* data, const std::size_t size) noexcept
    -> bool
{
    mode_ = Secret::Mode::Mem;

    return assign(data, size, effective_size(size, mode_));
}

auto SecretPrivate::AssignText(const ReadView source) noexcept -> bool
{
    mode_ = Secret::Mode::Text;

    return assign(
        source.data(), source.size(), effective_size(source.size(), mode_));
}

auto SecretPrivate::assign(
    const void* data,
    std::size_t copy,
    std::size_t reserve) noexcept -> bool
{
    OT_ASSERT(reserve >= copy);

    data_.clear();
    data_.reserve(reserve);
    data_.resize(reserve, {});
    std::memcpy(data_.data(), data, copy);

    return true;
}

auto SecretPrivate::resize(const std::size_t size) noexcept -> bool
{
    return ByteArrayPrivate::resize(effective_size(size, mode_));
}

auto SecretPrivate::SetSize(const std::size_t size) noexcept -> bool
{
    return ByteArrayPrivate::SetSize(effective_size(size, mode_));
}

auto SecretPrivate::size() const noexcept -> std::size_t
{
    const auto size = data_.size();

    if (mem()) {

        return size;
    } else {
        OT_ASSERT(0_uz < size);

        return size - 1_uz;
    }
}

auto SecretPrivate::WriteInto() noexcept -> Writer { return WriteInto(mode_); }

auto SecretPrivate::WriteInto(Secret::Mode mode) noexcept -> Writer
{
    return {
        [mode, this](auto size) -> WriteBuffer {
            mode_ = mode;

            if (false == resize(size)) {
                LogError()(OT_PRETTY_CLASS())("failed to resize to ")(
                    size)(" bytes")
                    .Flush();

                return std::span<std::byte>{};
            }

            if (const auto got = this->size(); got != size) {
                LogError()(OT_PRETTY_CLASS())("tried to reserve ")(
                    size)(" bytes but got ")(got)(" bytes")
                    .Flush();

                return std::span<std::byte>{};
            }

            return std::span<std::byte>{static_cast<std::byte*>(data()), size};
        },
        [this](auto size) -> bool {
            if (Secret::Mode::Text == mode_) { size += 1_uz; }

            data_.resize(size);

            if (Secret::Mode::Text == mode_) { data_.back() = {}; }

            return true;
        }};
}
}  // namespace opentxs
