// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <functional>
#include <ostream>
#include <source_location>
#include <span>
#include <type_traits>

#include "opentxs/Export.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Message;
}  // namespace internal

class Envelope;
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

class Amount;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::network::zeromq::Message> {
    using is_avalanching = void;

    auto operator()(const opentxs::network::zeromq::Message& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto operator==(const Message&, const Message&) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Message&, const Message&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto receive_to_message(
    std::ostream& logTo,
    void* socket,
    Message& msg,
    int flags = 0,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> bool;
OPENTXS_EXPORT auto receive_to_message(
    void* socket,
    Message& msg,
    int flags = 0) noexcept -> void;
OPENTXS_EXPORT auto reply_to_message(
    const Envelope& envelope,
    bool addDelimiter) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(
    Envelope&& envelope,
    bool addDelimiter) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(
    const Envelope& envelope,
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(
    Envelope&& envelope,
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(Message&& request) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(const Message& request) noexcept
    -> Message;
OPENTXS_EXPORT auto reply_to_message(
    const Message& request,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message;
OPENTXS_EXPORT auto send_from_message(
    std::ostream& logTo,
    Message&& msg,
    void* socket,
    int flags = 0,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> bool;
OPENTXS_EXPORT auto send_from_message(
    Message&& msg,
    void* socket,
    int flags = 0) noexcept -> void;
OPENTXS_EXPORT auto swap(Message& lhs, Message& rhs) noexcept -> void;
OPENTXS_EXPORT auto tagged_message(
    const void* tag,
    const std::size_t tagBytes,
    bool addDelimiter) noexcept -> Message;

class OPENTXS_EXPORT Message
{
public:
    class Imp;

    auto Envelope() const& noexcept -> zeromq::Envelope;
    auto get() const noexcept -> std::span<const Frame>;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Message&;
    auto Payload() const noexcept -> std::span<const Frame>;
    auto size() const noexcept -> std::size_t;

    auto AddFrame() noexcept -> Frame&;
    auto AddFrame(const Amount& amount) noexcept -> Frame&;
    auto AddFrame(Frame&& frame) noexcept -> Frame&;
    auto AddFrame(const char*) noexcept -> Frame&;
    template <
        typename Input,
        typename = std::enable_if_t<
            std::is_pointer<decltype(std::declval<Input&>().data())>::value>,
        typename = std::enable_if_t<
            std::is_integral<decltype(std::declval<Input&>().size())>::value>>
    auto AddFrame(const Input& input) noexcept -> Frame&
    {
        return AddFrame(input.data(), input.size());
    }
    template <
        typename Input,
        typename = std::enable_if_t<std::is_trivially_copyable<Input>::value>>
    auto AddFrame(const Input& input) noexcept -> Frame&
    {
        return AddFrame(&input, sizeof(input));
    }
    auto AddFrame(const void* input, const std::size_t size) noexcept -> Frame&;
    auto AppendBytes() noexcept -> Writer;
    auto CopyFrames(std::span<const Frame> frames) noexcept -> void;
    auto Envelope() && noexcept -> zeromq::Envelope;
    auto ExtractFront() noexcept -> zeromq::Frame;
    auto get() noexcept -> std::span<Frame>;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Message&;
    auto MoveFrames(std::span<Frame> frames) noexcept -> void;
    auto Payload() noexcept -> std::span<Frame>;
    auto Prepend(ReadView frame) noexcept -> zeromq::Frame&;
    auto Prepend(zeromq::Frame frame) noexcept -> zeromq::Frame&;
    auto StartBody() noexcept -> void;
    virtual auto swap(Message& rhs) noexcept -> void;

    Message() noexcept;
    OPENTXS_NO_EXPORT Message(Imp* imp) noexcept;
    Message(const Message&) noexcept;
    Message(Message&&) noexcept;
    auto operator=(const Message&) noexcept -> Message&;
    auto operator=(Message&&) noexcept -> Message&;

    virtual ~Message();

protected:
    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
