// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Pipeline;
}  // namespace internal

class Context;
class Message;
class Pipeline;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Pipeline final : virtual public Allocated
{
public:
    class Imp;

    auto BatchID() const noexcept -> std::size_t;
    auto BindSubscriber(
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept -> bool;
    auto Close() const noexcept -> bool;
    auto ConnectDealer(
        const std::string_view endpoint,
        std::function<Message(bool)> notify = {}) const noexcept -> bool;
    auto ConnectionIDDealer() const noexcept -> std::size_t;
    auto ConnectionIDInternal() const noexcept -> std::size_t;
    auto ConnectionIDPull() const noexcept -> std::size_t;
    auto ConnectionIDSubscribe() const noexcept -> std::size_t;
    auto get_allocator() const noexcept -> alloc::Default final;
    auto Internal() const noexcept -> const internal::Pipeline&;
    auto PullFrom(const std::string_view endpoint) const noexcept -> bool;
    auto Push(Message&& msg) const noexcept -> bool;
    auto Send(Message&& msg) const noexcept -> bool;
    auto SubscribeTo(const std::string_view endpoint) const noexcept -> bool;

    auto Internal() noexcept -> internal::Pipeline&;

    Pipeline(Imp* imp) noexcept;
    Pipeline() = delete;
    Pipeline(const Pipeline&) = delete;
    Pipeline(Pipeline&& rhs) noexcept;
    auto operator=(Pipeline&& rhs) noexcept -> Pipeline& = delete;
    auto operator=(const Pipeline&) -> Pipeline& = delete;

    ~Pipeline() final;

private:
    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
