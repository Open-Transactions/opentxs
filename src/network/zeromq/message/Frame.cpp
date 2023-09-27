// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/message/Frame.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <compare>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Spaceship.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto ZMQFrame(const std::size_t size) noexcept -> network::zeromq::Frame
{
    using ReturnType = network::zeromq::Frame;

    return std::make_unique<ReturnType::Imp>(size).release();
}

auto ZMQFrame(const void* data, const std::size_t size) noexcept
    -> network::zeromq::Frame
{
    using ReturnType = network::zeromq::Frame;

    return std::make_unique<ReturnType::Imp>(data, size).release();
}

auto ZMQFrame(const ProtobufType& data) noexcept -> network::zeromq::Frame
{
    using ReturnType = network::zeromq::Frame;

    return std::make_unique<ReturnType::Imp>(data).release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq
{
auto operator==(const Frame& lhs, const Frame& rhs) noexcept -> bool
{
    return lhs.Bytes() == rhs.Bytes();
}

auto operator==(std::span<const Frame> lhs, std::span<const Frame> rhs) noexcept
    -> bool
{
    const auto count = lhs.size();

    if (rhs.size() != count) { return false; }

    for (auto n = 0_uz; n < count; ++n) {
        if (lhs[n] != rhs[n]) { return false; }
    }

    return true;
}

auto operator<=>(const Frame& lhs, const Frame& rhs) noexcept
    -> std::strong_ordering
{
    return llvm_sucks(lhs.Bytes(), rhs.Bytes());
}

auto operator<=>(
    std::span<const Frame> lhs,
    std::span<const Frame> rhs) noexcept -> std::strong_ordering
{
    if (auto s1 = lhs.size(), s2 = rhs.size(); s1 < s2) {

        return std::strong_ordering::less;
    } else if (s2 < s1) {

        return std::strong_ordering::greater;
    } else {
        constexpr auto same = std::strong_ordering::equal;

        for (auto n = 0_uz; n < s1; ++n) {
            // TODO revisit this after libc++ in Android NDK is less broken
            // if (auto val = lhs[n] <=> rhs[n]; same != val) { return val; }
            if (lhs[n] == rhs[n]) {

                continue;
            } else {

                return lhs[n] <=> rhs[n];
            }
        }

        return same;
    }
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Frame::Imp::Imp(const void* data, std::size_t size) noexcept
    : message_()
{
    const auto init = ::zmq_msg_init_size(&message_, size);

    OT_ASSERT(0 == init);
    OT_ASSERT(size <= std::numeric_limits<int>::max());

    if ((0u < size) && (nullptr != data)) {
        std::memcpy(::zmq_msg_data(&message_), data, size);
    }
}

Frame::Imp::Imp(std::size_t size) noexcept
    : Imp(nullptr, size)
{
}

Frame::Imp::Imp() noexcept
    : Imp(0u)
{
}

Frame::Imp::Imp(const ProtobufType& input) noexcept
    : Imp(input.ByteSizeLong())
{
    input.SerializeToArray(data(), static_cast<int>(size()));
}

Frame::Imp::Imp(const Imp& rhs) noexcept
    : Imp(rhs.data(), rhs.size())
{
}

auto Frame::Imp::operator<(const zeromq::Frame& rhs) const noexcept -> bool
{
    const auto cmp =
        std::memcmp(data(), rhs.data(), std::min(size(), rhs.size()));

    if (0 > cmp) {

        return true;
    } else if (0 < cmp) {

        return false;
    } else {

        return size() < rhs.size();
    }
}

auto Frame::Imp::operator==(const zeromq::Frame& rhs) const noexcept -> bool
{
    return (size() == rhs.size()) &&
           (0 == std::memcmp(data(), rhs.data(), std::min(size(), rhs.size())));
}

Frame::Imp::~Imp() { ::zmq_msg_close(&message_); }
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Frame::Frame(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Frame::Frame() noexcept
    : Frame(std::make_unique<Imp>().release())
{
}

Frame::Frame(const Frame& rhs) noexcept
    : Frame(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Frame::Frame(Frame&& rhs) noexcept
    : Frame()
{
    swap(rhs);
}

auto Frame::operator=(const Frame& rhs) noexcept -> Frame&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();

    return *this;
}

auto Frame::operator=(Frame&& rhs) noexcept -> Frame&
{
    swap(rhs);

    return *this;
}

Frame::operator zmq_msg_t*() noexcept { return *imp_; }

auto Frame::Bytes() const noexcept -> ReadView { return imp_->Bytes(); }

auto Frame::data() const noexcept -> const void* { return imp_->data(); }

auto Frame::Internal() const noexcept -> const internal::Frame&
{
    return *imp_;
}

auto Frame::Internal() noexcept -> internal::Frame& { return *imp_; }

auto Frame::operator+=(const Frame& rhs) noexcept -> Frame&
{
    const auto lSize = size();
    const auto rSize = rhs.size();
    auto other = Frame{std::make_unique<Imp>(lSize + rSize).release()};
    auto* i = static_cast<std::byte*>(other.imp_->data());
    std::memcpy(i, data(), lSize);
    std::advance(i, lSize);
    std::memcpy(i, rhs.data(), rSize);
    swap(other);

    return *this;
}

auto Frame::size() const noexcept -> std::size_t { return imp_->size(); }

auto Frame::swap(Frame& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Frame::WriteInto() noexcept -> Writer
{
    return {[this](auto size) -> WriteBuffer {
        auto rhs = Frame{std::make_unique<Imp>(size).release()};
        swap(rhs);
        auto* out = imp_->data();

        return std::span<std::byte>{out, imp_->size()};
    }};
}

Frame::~Frame()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq
