// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/message/Message.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <compare>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <source_location>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/network/zeromq/message/Factory.hpp"
#include "internal/network/zeromq/message/Frame.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "network/zeromq/message/EnvelopePrivate.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Types.internal.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::zeromq
{
auto operator==(const Message& lhs, const Message& rhs) noexcept -> bool
{
    return lhs.get() == rhs.get();
}

auto operator<=>(const Message& lhs, const Message& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.get() <=> rhs.get();
}

static auto receive_to_message(
    void* socket,
    Message& msg,
    int flags,
    bool logErrors,
    const std::source_location& loc = {},
    std::ostream& logTo = std::cerr) noexcept -> bool
{
    try {
        auto more = int{};
        auto moreBytes = sizeof(more);

        do {
            auto frame = Frame{};
            auto rc = ::zmq_msg_recv(frame, socket, flags);

            if (-1 == rc) {
                throw std::runtime_error{::zmq_strerror(::zmq_errno())};
            }

            rc = ::zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &moreBytes);

            if (0 != rc) {
                throw std::runtime_error{::zmq_strerror(::zmq_errno())};
            }

            msg.AddFrame(std::move(frame));
        } while (0 != more);

        return true;
    } catch (const std::exception& e) {
        if (logErrors) {
            logTo << loc.function_name() << " in " << loc.file_name() << ": "
                  << e.what() << std::endl;

            return false;
        } else {

            return true;
        }
    }
}

auto receive_to_message(
    std::ostream& logTo,
    void* socket,
    Message& msg,
    int flags,
    const std::source_location& loc) noexcept -> bool
{
    return receive_to_message(socket, msg, flags, true, loc, logTo);
}

auto receive_to_message(void* socket, Message& msg, int flags) noexcept -> void
{
    receive_to_message(socket, msg, flags, false);
}

auto reply_to_message(const Envelope& envelope, bool addDelimiter) noexcept
    -> Message
{
    auto output = Message{};
    output.CopyFrames(envelope.get());

    if (addDelimiter) { output.StartBody(); }

    return output;
}

auto reply_to_message(Envelope&& envelope, bool addDelimiter) noexcept
    -> Message
{
    auto output = Message{};
    output.MoveFrames(envelope.get());

    if (addDelimiter) { output.StartBody(); }

    return output;
}

auto reply_to_message(
    const Envelope& envelope,
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message
{
    auto output = reply_to_message(envelope, addDelimiter);
    output.AddFrame(tag, tagBytes);

    return output;
}

auto reply_to_message(
    Envelope&& envelope,
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message
{
    auto output = reply_to_message(std::move(envelope), addDelimiter);
    output.AddFrame(tag, tagBytes);

    return output;
}

auto reply_to_message(Message&& request) noexcept -> Message
{
    auto envelope = std::move(request).Envelope();
    const auto addDelimiter = envelope.IsValid();

    return reply_to_message(std::move(envelope), addDelimiter);
}

auto reply_to_message(const Message& request) noexcept -> Message
{
    auto envelope = request.Envelope();
    const auto addDelimiter = envelope.IsValid();

    return reply_to_message(std::move(envelope), addDelimiter);
}

auto reply_to_message(
    const Message& request,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message
{
    auto envelope = request.Envelope();
    const auto addDelimiter = envelope.IsValid();

    return reply_to_message(std::move(envelope), tag, tagBytes, addDelimiter);
}

static auto send_from_message(
    Message&& msg,
    void* socket,
    int flags,
    bool logErrors,
    const std::source_location& loc = {},
    std::ostream& logTo = std::cerr) noexcept -> bool
{
    try {
        const auto transmit = [=](::zmq_msg_t* part, int f) {
            if (-1 == ::zmq_msg_send(part, socket, f)) {
                throw std::runtime_error{::zmq_strerror(::zmq_errno())};
            }
        };
        const auto transmit_more = [=](::zmq_msg_t* part) {
            transmit(part, flags | ZMQ_SNDMORE);
        };
        const auto transmit_last = [=](::zmq_msg_t* part) {
            transmit(part, flags);
        };
        auto frames = msg.get();
        const auto count = frames.size();

        if (0_uz == count) { return true; }

        const auto last = count - 1_uz;

        for (auto n = 0_uz; n < last; ++n) { transmit_more(frames[n]); }

        transmit_last(frames[last]);

        return true;
    } catch (const std::exception& e) {
        if (logErrors) {
            logTo << loc.function_name() << " in " << loc.file_name() << ": "
                  << e.what() << std::endl;

            return false;
        } else {

            return true;
        }
    }
}

auto send_from_message(
    std::ostream& logTo,
    Message&& msg,
    void* socket,
    int flags,
    const std::source_location& loc) noexcept -> bool
{
    return send_from_message(std::move(msg), socket, flags, true, loc, logTo);
}

auto send_from_message(Message&& msg, void* socket, int flags) noexcept -> void
{
    send_from_message(std::move(msg), socket, flags, false);
}

auto swap(Message& lhs, Message& rhs) noexcept -> void { return lhs.swap(rhs); }

auto tagged_message(
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message
{
    auto output = Message();

    if (addDelimiter) { output.StartBody(); }

    output.AddFrame(tag, tagBytes);

    return output;
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Message::Imp::Imp() noexcept
    : parent_(nullptr)
    , frames_()
    , delimiter_(std::nullopt)
{
}

Message::Imp::Imp(const Imp& rhs) noexcept
    : parent_(nullptr)
    , frames_(rhs.frames_)
    , delimiter_(rhs.delimiter_)
{
}

auto Message::Imp::AddFrame() noexcept -> Frame&
{
    return AddFrame(factory::ZMQFrame(0_uz));
}

auto Message::Imp::AddFrame(const Amount& amount) noexcept -> Frame&
{
    amount.Serialize(AppendBytes());

    return frames_.back();
}

auto Message::Imp::AddFrame(Frame&& frame) noexcept -> Frame&
{
    if (false == delimiter_.has_value() && (0_uz == frame.size())) {
        delimiter_.emplace(frames_.size());
    }

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
    return AddFrame(factory::ZMQFrame(input, size));
}

auto Message::Imp::AddFrame(const ProtobufType& input) noexcept -> Frame&
{
    return AddFrame(factory::ZMQFrame(input));
}

auto Message::Imp::AppendBytes() noexcept -> Writer
{
    return {[this](const std::size_t size) -> WriteBuffer {
        auto& frame = AddFrame(factory::ZMQFrame(size));
        auto* out = frame.Internal().data();

        return std::span<std::byte>{out, frame.size()};
    }};
}

// This function is only called by RouterSocket.  It makes sure that if a
// message has two or more frames, and no delimiter, then a delimiter is
// inserted after the first frame.
auto Message::Imp::EnsureDelimiter() noexcept -> void
{
    switch (frames_.size()) {
        case 0_uz: {
            frames_.emplace_back();
            delimiter_ = 0_uz;
        } break;
        case 1_uz: {
            if (false == delimiter_.has_value()) {
                frames_.emplace(frames_.begin());
                delimiter_.emplace(0_uz);
            }
        } break;
        case 2_uz:
        default: {
            if (false == delimiter_.has_value()) {
                auto it = frames_.begin();
                frames_.emplace(++it);
                delimiter_.emplace(1_uz);
            }
        }
    }
}

auto Message::Imp::Envelope() const noexcept -> std::span<const Frame>
{
    if (delimiter_.has_value()) {
        if (0_uz < *delimiter_) {

            return {std::addressof(frames_.front()), *delimiter_};
        } else {

            return {};
        }
    } else {

        return {};
    }
}

auto Message::Imp::Envelope() noexcept -> std::span<Frame>
{
    if (delimiter_.has_value()) {
        if (0_uz < *delimiter_) {

            return {std::addressof(frames_.front()), *delimiter_};
        } else {

            return {};
        }
    } else {

        return {};
    }
}

auto Message::Imp::ExtractFront() noexcept -> zeromq::Frame
{
    auto output = zeromq::Frame{};

    if (false == frames_.empty()) {
        auto it = frames_.begin();
        output.swap(*it);
        frames_.erase(it);

        if (delimiter_.has_value()) {
            if (*delimiter_ > 0_uz) {
                --*delimiter_;
            } else {
                delimiter_ = std::nullopt;
            }
        }
    }

    return output;
}

auto Message::Imp::Payload() const noexcept -> std::span<const Frame>
{
    if (delimiter_.has_value()) {
        const auto start = *delimiter_ + 1_uz;
        const auto count = frames_.size();

        if (start < count) {

            return {std::addressof(frames_[start]), count - start};
        } else {

            return {};
        }
    } else {

        return get();
    }
}

auto Message::Imp::Payload() noexcept -> std::span<Frame>
{
    if (delimiter_.has_value()) {
        const auto start = *delimiter_ + 1_uz;
        const auto count = frames_.size();

        if (start < count) {

            return {std::addressof(frames_[start]), count - start};
        } else {

            return {};
        }
    } else {

        return get();
    }
}

auto Message::Imp::Prepend(ReadView frame) noexcept -> zeromq::Frame&
{
    return Prepend(factory::ZMQFrame(frame.data(), frame.size()));
}

auto Message::Imp::Prepend(zeromq::Frame frame) noexcept -> zeromq::Frame&
{
    if (frames_.empty()) {
        AddFrame(std::move(frame));
        StartBody();
    } else if (delimiter_.has_value()) {
        frames_.emplace(frames_.begin(), std::move(frame));
        ++*delimiter_;
    } else {
        frames_.emplace(frames_.begin());
        frames_.emplace(frames_.begin(), std::move(frame));
        delimiter_.emplace(1_uz);
    }

    return frames_.front();
}

auto Message::Imp::Prepend(SocketID id) noexcept -> zeromq::Frame&
{
    return Prepend(factory::ZMQFrame(&id, sizeof(id)));
}

auto Message::Imp::set_field(
    const std::size_t position,
    const zeromq::Frame& input) noexcept -> bool
{
    const auto bodyStart = [&] {
        if (delimiter_.has_value()) {

            return *delimiter_ + 1_uz;
        } else {

            return 0_uz;
        }
    }();
    const auto effectivePosition = bodyStart + position;

    if (effectivePosition >= frames_.size()) { return false; }

    auto& frame = frames_[effectivePosition];
    frame = input;

    return true;
}

auto Message::Imp::StartBody() noexcept -> void
{
    if (false == delimiter_.has_value()) { AddFrame(); }
}

auto Message::Imp::size() const noexcept -> std::size_t
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
    assert_false(nullptr == imp_);

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

auto Message::AppendBytes() noexcept -> Writer { return imp_->AppendBytes(); }

auto Message::CopyFrames(std::span<const Frame> frames) noexcept -> void
{
    for (const auto& frame : frames) { AddFrame(frame); }
}

auto Message::Envelope() const& noexcept -> zeromq::Envelope
{
    auto alloc = alloc::Strategy{};  // TODO function argument

    return pmr::construct<EnvelopePrivate>(alloc.result_, *this);
}

auto Message::Envelope() && noexcept -> zeromq::Envelope
{
    auto alloc = alloc::Strategy{};  // TODO function argument

    return pmr::construct<EnvelopePrivate>(alloc.result_, std::move(*this));
}

auto Message::ExtractFront() noexcept -> zeromq::Frame
{
    return imp_->ExtractFront();
}

auto Message::get() const noexcept -> std::span<const Frame>
{
    return imp_->get();
}

auto Message::get() noexcept -> std::span<Frame> { return imp_->get(); }

auto Message::Internal() const noexcept -> const internal::Message&
{
    return *imp_;
}

auto Message::Internal() noexcept -> internal::Message& { return *imp_; }

auto Message::MoveFrames(std::span<Frame> frames) noexcept -> void
{
    for (auto& frame : frames) { AddFrame(std::move(frame)); }
}

auto Message::Payload() const noexcept -> std::span<const Frame>
{
    return imp_->Payload();
}

auto Message::Payload() noexcept -> std::span<Frame> { return imp_->Payload(); }

auto Message::Prepend(ReadView frame) noexcept -> zeromq::Frame&
{
    return imp_->Prepend(frame);
}

auto Message::Prepend(zeromq::Frame frame) noexcept -> zeromq::Frame&
{
    return imp_->Prepend(std::move(frame));
}

auto Message::StartBody() noexcept -> void { return imp_->StartBody(); }

auto Message::swap(Message& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
    std::swap(imp_->parent_, rhs.imp_->parent_);
}

auto Message::size() const noexcept -> std::size_t { return imp_->size(); }

Message::~Message()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq
