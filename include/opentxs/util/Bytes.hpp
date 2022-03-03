// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/util/Container.hpp"

namespace opentxs
{
template <typename ViewType>
class ViewWrapper
{
public:
    operator bool() const noexcept { return valid(); }

    auto bytes() const noexcept -> const std::byte*
    {
        return static_cast<const std::byte*>(data());
    }
    auto data() const noexcept -> const void* { return view_.data(); }
    auto get() const noexcept -> const ViewType& { return view_; }
    auto size() const noexcept { return view_.size(); }
    auto valid() const noexcept { return (nullptr != data()) && (0 != size()); }

    auto get() noexcept -> ViewType& { return view_; }

    ViewWrapper(ViewType&& view) noexcept
        : view_(std::move(view))
    {
    }
    ViewWrapper() noexcept = default;
    ViewWrapper(const ViewWrapper&) = delete;
    ViewWrapper(ViewWrapper&&) = default;
    auto operator=(const ViewWrapper&) -> ViewWrapper& = delete;
    auto operator=(ViewWrapper&&) -> ViewWrapper& = default;
    virtual ~ViewWrapper() = default;

private:
    ViewType view_{};
};

template <typename ViewType, typename MutexType, typename LockType>
class ProtectedView : virtual public ViewWrapper<ViewType>
{
public:
    using DestructCallback = std::function<void()>;

    ProtectedView(
        ViewType&& view,
        MutexType& mutex,
        DestructCallback cb = {}) noexcept
        : ViewWrapper<ViewType>(std::move(view))
        , lock_(std::make_unique<LockType>(mutex))
        , cb_(cb)
    {
    }
    ProtectedView() noexcept = default;
    ProtectedView(const ProtectedView&) = delete;
    ProtectedView(ProtectedView&&) = default;
    auto operator=(const ProtectedView&) -> ProtectedView& = delete;
    auto operator=(ProtectedView&&) -> ProtectedView& = default;
    ~ProtectedView() override
    {
        lock_.reset();

        if (cb_) { cb_(); }
    }

private:
    std::unique_ptr<const LockType> lock_{};
    DestructCallback cb_{};
};

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
    WritableView(WritableView&& rhs) noexcept
        : data_(std::move(rhs.data_))
        , size_(std::move(rhs.size_))
    {
    }

    ~WritableView() = default;

private:
    void* data_;
    std::size_t size_;

    WritableView(const WritableView&) = delete;
    auto operator=(WritableView&&) -> WritableView& = delete;
};

using AllocateOutput = std::function<WritableView(const std::size_t)>;
using Space = UnallocatedVector<std::byte>;

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
OPENTXS_EXPORT auto reader(const UnallocatedVector<std::uint8_t>& in) noexcept
    -> ReadView;
OPENTXS_EXPORT auto space(const std::size_t size) noexcept -> Space;
OPENTXS_EXPORT auto space(const ReadView bytes) noexcept -> Space;
OPENTXS_EXPORT auto valid(const ReadView view) noexcept -> bool;
OPENTXS_EXPORT auto writer(UnallocatedCString& in) noexcept -> AllocateOutput;
OPENTXS_EXPORT auto writer(UnallocatedCString* protobuf) noexcept
    -> AllocateOutput;
OPENTXS_EXPORT auto writer(Space& in) noexcept -> AllocateOutput;
}  // namespace opentxs
