// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <type_traits>

#include "opentxs/Export.hpp"
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

class Frame;
class FrameIterator;
class FrameSection;
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
struct hash<opentxs::network::zeromq::Message>;
}  // namespace std

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto operator<(const Message& lhs, const Message& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator==(const Message& lhs, const Message& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto reply_to_connection(const ReadView connectionID) noexcept
    -> Message;
OPENTXS_EXPORT auto reply_to_connection(
    const ReadView connectionID,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(Message&& request) noexcept -> Message;
OPENTXS_EXPORT auto reply_to_message(const Message& request) noexcept
    -> Message;
OPENTXS_EXPORT auto reply_to_message(
    const Message& request,
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message;
OPENTXS_EXPORT auto swap(Message& lhs, Message& rhs) noexcept -> void;
OPENTXS_EXPORT auto tagged_message(
    const void* tag,
    const std::size_t tagBytes) noexcept -> Message;

class OPENTXS_EXPORT Message
{
public:
    class Imp;

    auto at(const std::size_t index) const noexcept(false) -> const Frame&;
    auto begin() const noexcept -> const FrameIterator;
    auto Body() const noexcept -> const FrameSection;
    auto Body_at(const std::size_t index) const noexcept(false) -> const Frame&;
    auto Body_begin() const noexcept -> const FrameIterator;
    auto Body_end() const noexcept -> const FrameIterator;
    auto end() const noexcept -> const FrameIterator;
    auto Header() const noexcept -> const FrameSection;
    auto Header_at(const std::size_t index) const noexcept(false)
        -> const Frame&;
    auto Header_begin() const noexcept -> const FrameIterator;
    auto Header_end() const noexcept -> const FrameIterator;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Message&;
    auto size() const noexcept -> std::size_t;
    auto Total() const noexcept -> std::size_t;

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
    auto at(const std::size_t index) noexcept(false) -> Frame&;
    auto Body() noexcept -> FrameSection;
    auto EnsureDelimiter() noexcept -> void;
    auto Header() noexcept -> FrameSection;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Message&;
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
    friend auto zeromq::operator<(const Message&, const Message&) noexcept
        -> bool;
    friend auto zeromq::operator==(const Message&, const Message&) noexcept
        -> bool;

    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
