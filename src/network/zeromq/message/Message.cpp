// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "network/zeromq/message/Message.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>

#include "internal/network/zeromq/message/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "network/zeromq/message/FrameIterator.hpp"
#include "network/zeromq/message/FrameSection.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::network::zeromq
{
auto operator<(const Message& lhs, const Message& rhs) noexcept -> bool
{
    return lhs.imp_->operator<(rhs);
}

auto operator==(const Message& lhs, const Message& rhs) noexcept -> bool
{
    return lhs.imp_->operator==(rhs);
}

auto reply_to_connection(const ReadView id) noexcept -> Message
{
    auto output = Message{};
    output.AddFrame(id.data(), id.size());
    output.StartBody();

    return output;
}

auto reply_to_connection(
    const ReadView connectionID,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message
{
    auto output = reply_to_connection(connectionID);
    output.StartBody();
    output.AddFrame(tag, tagBytes);

    return output;
}

auto reply_to_message(Message&& request) noexcept -> Message
{
    auto output = Message{};

    if (0 < request.Header().size()) {
        for (auto& frame : request.Header()) {
            output.AddFrame(std::move(frame));
        }

        output.StartBody();
    }

    return output;
}

auto reply_to_message(const Message& request) noexcept -> Message
{
    auto output = Message{};

    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) { output.AddFrame(frame); }

        output.StartBody();
    }

    return output;
}

auto reply_to_message(
    const Message& request,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message
{
    auto output = reply_to_message(request);
    output.StartBody();
    output.AddFrame(tag, tagBytes);

    return output;
}

auto swap(Message& lhs, Message& rhs) noexcept -> void { return lhs.swap(rhs); }

auto tagged_message(const void* tag, const std::size_t tagBytes) noexcept
    -> Message
{
    auto output = Message();
    output.StartBody();
    output.AddFrame(tag, tagBytes);

    return output;
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Message::Imp::Imp() noexcept
    : parent_(nullptr)
    , frames_()
{
}

Message::Imp::Imp(const Imp& rhs) noexcept
    : parent_(nullptr)
    , frames_(rhs.frames_)
{
}

auto Message::Imp::AddFrame() noexcept -> Frame&
{
    return frames_.emplace_back();
}

auto Message::Imp::AddFrame(const Amount& amount) noexcept -> Frame&
{
    amount.Serialize(AppendBytes());

    return frames_.back();
}

auto Message::Imp::AddFrame(Frame&& frame) noexcept -> Frame&
{
    return frames_.emplace_back(std::move(frame));
}

auto Message::Imp::AddFrame(const char* in) noexcept -> Frame&
{
    const auto string = UnallocatedCString{in};

    return AddFrame(string.data(), string.size());
}

auto Message::Imp::AddFrame(const ReadView bytes) noexcept -> Frame&
{
    return AddFrame(bytes.data(), bytes.size());
}

auto Message::Imp::AddFrame(const void* input, const std::size_t size) noexcept
    -> Frame&
{
    return frames_.emplace_back(factory::ZMQFrame(input, size));
}

auto Message::Imp::AddFrame(const ProtobufType& input) noexcept -> Frame&
{
    return frames_.emplace_back(factory::ZMQFrame(input));
}

auto Message::Imp::AppendBytes() noexcept -> AllocateOutput
{
    return [this](const std::size_t size) -> WritableView {
        auto& frame = frames_.emplace_back(factory::ZMQFrame(size));

        return {const_cast<void*>(frame.data()), frame.size()};
    };
}

auto Message::Imp::at(const std::size_t index) const noexcept(false)
    -> const Frame&
{
    OT_ASSERT(frames_.size() > index);

    return const_cast<const Frame&>(frames_.at(index));
}

auto Message::Imp::at(const std::size_t index) noexcept(false) -> Frame&
{
    OT_ASSERT(frames_.size() > index);

    return frames_.at(index);
}

auto Message::Imp::begin() const noexcept -> const FrameIterator
{
    return FrameIterator{std::make_unique<FrameIterator::Imp>(
                             const_cast<zeromq::Message*>(parent_))
                             .release()};
}

auto Message::Imp::Body() const noexcept -> const FrameSection
{
    auto position = body_position();

    return std::make_unique<implementation::FrameSection>(
               parent_, position, frames_.size() - position)
        .release();
}

auto Message::Imp::Body() noexcept -> FrameSection
{
    auto position = body_position();

    return std::make_unique<implementation::FrameSection>(
               parent_, position, frames_.size() - position)
        .release();
}

auto Message::Imp::Body_at(const std::size_t index) const noexcept(false)
    -> const Frame&
{
    return Body().at(index);
}

auto Message::Imp::Body_begin() const noexcept -> const FrameIterator
{
    return Body().begin();
}

auto Message::Imp::Body_end() const noexcept -> const FrameIterator
{
    return Body().end();
}

auto Message::Imp::body_position() const noexcept -> std::size_t
{
    std::size_t position{0};

    if (true == hasDivider()) { position = findDivider() + 1; }

    return position;
}

auto Message::Imp::end() const noexcept -> const FrameIterator
{
    return FrameIterator{
        std::make_unique<FrameIterator::Imp>(
            const_cast<zeromq::Message*>(parent_), frames_.size())
            .release()};
}

// This function is only called by RouterSocket.  It makes sure that if a
// message has two or more frames, and no delimiter, then a delimiter is
// inserted after the first frame.
auto Message::Imp::EnsureDelimiter() noexcept -> void
{
    if (1 < frames_.size() && !hasDivider()) {
        auto it = frames_.begin();
        frames_.emplace(++it, Frame{});
    }
    // These cases should never happen.  When parent_ function is called, there
    // should always be at least two frames.
    else if (0 < frames_.size() && !hasDivider()) {
        frames_.emplace(frames_.begin(), Frame{});
    } else if (!hasDivider()) {
        frames_.emplace_back();
    }
}

auto Message::Imp::ExtractFront() noexcept -> zeromq::Frame
{
    auto output = zeromq::Frame{};

    if (0u < frames_.size()) {
        auto it = frames_.begin();
        output.swap(*it);
        frames_.erase(it);
    }

    return output;
}

auto Message::Imp::findDivider() const noexcept -> std::size_t
{
    std::size_t divider = 0;

    for (const auto& frame : frames_) {
        if (0 == frame.size()) { break; }
        ++divider;
    }

    return divider;
}

auto Message::Imp::hasDivider() const noexcept -> bool
{
    return std::find_if(
               frames_.begin(), frames_.end(), [](const Frame& frame) -> bool {
                   return 0 == frame.size();
               }) != frames_.end();
}

auto Message::Imp::Header_at(const std::size_t index) const noexcept(false)
    -> const Frame&
{
    return Header().at(index);
}

auto Message::Imp::Header() const noexcept -> const FrameSection
{
    auto size = 0_uz;

    if (true == hasDivider()) { size = findDivider(); }

    return std::make_unique<implementation::FrameSection>(parent_, 0, size)
        .release();
}

auto Message::Imp::Header() noexcept -> FrameSection
{
    auto size = 0_uz;

    if (true == hasDivider()) { size = findDivider(); }

    return std::make_unique<implementation::FrameSection>(parent_, 0, size)
        .release();
}

auto Message::Imp::Header_begin() const noexcept -> const FrameIterator
{
    return Header().begin();
}

auto Message::Imp::Header_end() const noexcept -> const FrameIterator
{
    return Header().end();
}

auto Message::Imp::operator<(const zeromq::Message& rhs) const noexcept -> bool
{
    const auto lEnd = end();
    const auto rEnd = rhs.end();

    for (auto l{begin()}, r{rhs.begin()}; (l != lEnd) && (r != rEnd);
         ++l, ++r) {
        if (*l < *r) { return true; }
    }

    return size() < rhs.size();
}

auto Message::Imp::operator==(const zeromq::Message& rhs) const noexcept -> bool
{
    if (size() != rhs.size()) { return false; }

    const auto lEnd = end();
    const auto rEnd = rhs.end();

    for (auto l{begin()}, r{rhs.begin()}; (l != lEnd) && (r != rEnd);
         ++l, ++r) {
        if (!(*l == *r)) { return false; }
    }

    return true;
}

auto Message::Imp::Prepend(SocketID id) noexcept -> zeromq::Frame&
{
    if (0u == frames_.size()) {
        AddFrame(&id, sizeof(id));
        AddFrame();
    } else {
        if (false == hasDivider()) {
            frames_.emplace(frames_.begin(), Frame{});
        }

        frames_.emplace(frames_.begin(), factory::ZMQFrame(&id, sizeof(id)));
    }

    return frames_.front();
}

auto Message::Imp::set_field(
    const std::size_t position,
    const zeromq::Frame& input) noexcept -> bool
{
    const auto effectivePosition = body_position() + position;

    if (effectivePosition >= frames_.size()) { return false; }

    auto& frame = frames_[effectivePosition];

    frame = input;

    return true;
}

auto Message::Imp::StartBody() noexcept -> void
{
    if (0 == frames_.size()) {
        frames_.emplace_back();
    } else if (0 < (*frames_.crbegin()).size()) {
        frames_.emplace_back();
    }
}

auto Message::Imp::size() const noexcept -> std::size_t
{
    return frames_.size();
}

auto Message::Imp::Total() const noexcept -> std::size_t
{
    return std::accumulate(
        frames_.begin(),
        frames_.end(),
        0_uz,
        [](const auto& lhs, const auto& rhs) { return lhs + rhs.size(); });
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Message::Message(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);

    imp_->parent_ = this;
}

Message::Message() noexcept
    : Message(std::make_unique<Imp>().release())
{
}

Message::Message(const Message& rhs) noexcept
    : Message(std::make_unique<Imp>(*rhs.imp_).release())
{
}

Message::Message(Message&& rhs) noexcept
    : Message()
{
    swap(rhs);
}

auto Message::operator=(const Message& rhs) noexcept -> Message&
{
    auto old = std::make_unique<Imp>(*imp_);
    imp_ = std::make_unique<Imp>(*rhs.imp_).release();
    imp_->parent_ = this;

    return *this;
}

auto Message::operator=(Message&& rhs) noexcept -> Message&
{
    swap(rhs);

    return *this;
}

auto Message::AddFrame() noexcept -> Frame& { return imp_->AddFrame(); }

auto Message::AddFrame(const Amount& in) noexcept -> Frame&
{
    return imp_->AddFrame(in);
}

auto Message::AddFrame(const char* in) noexcept -> Frame&
{
    return imp_->AddFrame(in);
}

auto Message::AddFrame(Frame&& frame) noexcept -> Frame&
{
    return imp_->AddFrame(std::move(frame));
}

auto Message::AddFrame(const void* input, const std::size_t size) noexcept
    -> Frame&
{
    return imp_->AddFrame(input, size);
}

auto Message::AppendBytes() noexcept -> AllocateOutput
{
    return imp_->AppendBytes();
}

auto Message::at(const std::size_t index) const noexcept(false) -> const Frame&
{
    return imp_->at(index);
}

auto Message::at(const std::size_t index) noexcept(false) -> Frame&
{
    return imp_->at(index);
}

auto Message::begin() const noexcept -> const FrameIterator
{
    return imp_->begin();
}

auto Message::Body() const noexcept -> const FrameSection
{
    return imp_->Body();
}

auto Message::Body() noexcept -> FrameSection { return imp_->Body(); }

auto Message::Body_at(const std::size_t index) const noexcept(false)
    -> const Frame&
{
    return imp_->Body_at(index);
}

auto Message::Body_begin() const noexcept -> const FrameIterator
{
    return imp_->Body_begin();
}

auto Message::Body_end() const noexcept -> const FrameIterator
{
    return imp_->Body_end();
}

auto Message::end() const noexcept -> const FrameIterator
{
    return imp_->end();
}

auto Message::EnsureDelimiter() noexcept -> void
{
    return imp_->EnsureDelimiter();
}

auto Message::Header_at(const std::size_t index) const noexcept(false)
    -> const Frame&
{
    return imp_->Header_at(index);
}

auto Message::Header() const noexcept -> const FrameSection
{
    return imp_->Header();
}

auto Message::Header() noexcept -> FrameSection { return imp_->Header(); }

auto Message::Header_begin() const noexcept -> const FrameIterator
{
    return imp_->Header_begin();
}

auto Message::Header_end() const noexcept -> const FrameIterator
{
    return imp_->Header_end();
}

auto Message::Internal() const noexcept -> const internal::Message&
{
    return *imp_;
}

auto Message::Internal() noexcept -> internal::Message& { return *imp_; }

auto Message::StartBody() noexcept -> void { return imp_->StartBody(); }

auto Message::size() const noexcept -> std::size_t { return imp_->size(); }

auto Message::swap(Message& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);
}

auto Message::Total() const noexcept -> std::size_t { return imp_->Total(); }

Message::~Message()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq
