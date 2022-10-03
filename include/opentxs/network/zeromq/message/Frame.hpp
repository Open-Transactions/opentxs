// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>  // IWYU pragma: keep
#include <functional>
#include <type_traits>

#include "opentxs/Export.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

struct zmq_msg_t;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Frame;
}  // namespace internal

class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::network::zeromq::Frame> {
    auto operator()(const opentxs::network::zeromq::Frame& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto swap(Frame& lhs, Frame& rhs) noexcept -> void;
OPENTXS_EXPORT auto operator<(const Frame& lhs, const Frame& rhs) noexcept
    -> bool;
OPENTXS_EXPORT auto operator==(const Frame& lhs, const Frame& rhs) noexcept
    -> bool;

class OPENTXS_EXPORT Frame
{
public:
    class Imp;

    template <
        typename Output,
        std::enable_if_t<std::is_trivially_copyable<Output>::value, int> = 0>
    auto as() const noexcept(false) -> Output
    {
        if (sizeof(Output) != size()) {
            auto error = UnallocatedCString{"Invalid frame size: "} +
                         std::to_string(size()) +
                         " expected: " + std::to_string(sizeof(Output));

            throw std::runtime_error(error);
        }

        auto output = Output{};
        std::memcpy(&output, data(), sizeof(output));

        return output;
    }

    auto Bytes() const noexcept -> ReadView;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Frame&;
    auto data() const noexcept -> const void*;
    auto size() const noexcept -> std::size_t;

    operator zmq_msg_t*() noexcept;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Frame&;
    auto operator+=(const Frame& rhs) noexcept -> Frame&;
    virtual auto swap(Frame& rhs) noexcept -> void;
    auto WriteInto() noexcept -> AllocateOutput;

    OPENTXS_NO_EXPORT Frame(Imp* imp) noexcept;
    Frame() noexcept;
    Frame(const Frame&) noexcept;
    Frame(Frame&&) noexcept;
    auto operator=(const Frame&) noexcept -> Frame&;
    auto operator=(Frame&&) noexcept -> Frame&;

    virtual ~Frame();

private:
    friend auto zeromq::operator<(const Frame&, const Frame&) noexcept -> bool;
    friend auto zeromq::operator==(const Frame&, const Frame&) noexcept -> bool;

    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
