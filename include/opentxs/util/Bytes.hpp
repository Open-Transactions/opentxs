// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
using ReadView = std::string_view;

class OPENTXS_EXPORT WritableView
{
public:
    operator void*() const noexcept { return data(); }
    operator std::size_t() const noexcept { return size(); }
    operator bool() const noexcept { return valid(); }
    operator ReadView() const noexcept
    {
        return {static_cast<const char*>(data_), size_};
    }

    template <typename DesiredType>
    auto as() const noexcept -> DesiredType*
    {
        return static_cast<DesiredType*>(data_);
    }

    auto data() const noexcept -> void* { return data_; }
    auto size() const noexcept -> std::size_t { return size_; }
    auto valid() const noexcept -> bool
    {
        return (nullptr != data_) && (0 != size_);
    }
    auto valid(const std::size_t size) const noexcept -> bool
    {
        return (nullptr != data_) && (size == size_);
    }

    WritableView(void* data, const std::size_t size) noexcept
        : data_(data)
        , size_(size)
    {
    }
    WritableView() noexcept
        : WritableView(nullptr, 0)
    {
    }
    WritableView(const WritableView&) = delete;
    WritableView(WritableView&& rhs) noexcept
        : data_(std::move(rhs.data_))
        , size_(std::move(rhs.size_))
    {
    }
    auto operator=(const WritableView&) -> WritableView& = delete;
    auto operator=(WritableView&& rhs) noexcept -> WritableView&
    {
        using std::swap;
        swap(data_, rhs.data_);
        swap(size_, rhs.size_);

        return *this;
    }

    ~WritableView() = default;

private:
    void* data_;
    std::size_t size_;
};

using AllocateOutput = std::function<WritableView(const std::size_t)>;
using Space = UnallocatedVector<std::byte>;
using RawData = UnallocatedVector<unsigned char>;

OPENTXS_EXPORT auto copy(const ReadView in, const AllocateOutput out) noexcept
    -> bool;
OPENTXS_EXPORT auto copy(
    const ReadView in,
    const AllocateOutput out,
    const std::size_t limit) noexcept -> bool;
OPENTXS_EXPORT auto preallocated(const std::size_t size, void* out) noexcept
    -> AllocateOutput;
OPENTXS_EXPORT auto reader(const WritableView& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const Space& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const Vector<std::byte>& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const UnallocatedVector<std::uint8_t>& in) noexcept
    -> ReadView;
template <std::size_t N>
OPENTXS_EXPORT auto reader(const std::array<std::byte, N>& in) noexcept
    -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), N};
}
OPENTXS_EXPORT auto space(const std::size_t size) noexcept -> Space;
OPENTXS_EXPORT auto space(const std::size_t size, alloc::Default alloc) noexcept
    -> Vector<std::byte>;
OPENTXS_EXPORT auto space(const ReadView bytes) noexcept -> Space;
OPENTXS_EXPORT auto space(const ReadView bytes, alloc::Default alloc) noexcept
    -> Vector<std::byte>;
OPENTXS_EXPORT auto valid(const ReadView view) noexcept -> bool;
OPENTXS_EXPORT auto writer(UnallocatedCString& in) noexcept -> AllocateOutput;
OPENTXS_EXPORT auto writer(UnallocatedCString* protobuf) noexcept
    -> AllocateOutput;
OPENTXS_EXPORT auto writer(Space& in) noexcept -> AllocateOutput;
OPENTXS_EXPORT auto writer(Vector<std::byte>& in) noexcept -> AllocateOutput;
template <std::size_t N>
OPENTXS_EXPORT auto writer(std::array<std::byte, N>& in) noexcept
    -> AllocateOutput
{
    return preallocated(N, in.data());
}
}  // namespace opentxs
