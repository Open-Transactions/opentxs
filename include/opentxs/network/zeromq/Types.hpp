// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <string_view>
#include <tuple>  // IWYU pragma: keep
#include <utility>

#include "opentxs/Export.hpp"                          // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Message.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::actor
{
using SocketIndex = std::size_t;
using Replies = Vector<std::pair<SocketIndex, Vector<Message>>>;

using Processor = std::function<
    Replies(SocketIndex, OTZMQWorkType, Message&&, alloc::Strategy)>;
using Shutdown = std::function<void()>;
/// If the startup function returns true then the actor will immediately
/// shutdown
using Startup = std::function<bool(alloc::Strategy)>;
/// If the state machine function returns true then it will be executed again
using StateMachine = std::function<bool(alloc::Strategy)>;

constexpr auto SubscribeIndex = SocketIndex{0};
constexpr auto PullIndex = SocketIndex{1};
constexpr auto DealerIndex = SocketIndex{2};
constexpr auto LoopbackIndex = SocketIndex{3};
constexpr auto FirstUserDefinedIndex = SocketIndex{4};
}  // namespace opentxs::network::zeromq::actor

namespace opentxs::network::zeromq
{
using BatchID = std::size_t;
using SocketID = std::size_t;

OPENTXS_EXPORT auto CurveKeypair(Writer&& sec, Writer&& pub) noexcept -> bool;
OPENTXS_EXPORT auto CurveKeypairZ85(Writer&& sec, Writer&& pub) noexcept
    -> bool;
OPENTXS_EXPORT auto DefaultProcessor() noexcept -> actor::Processor;
OPENTXS_EXPORT auto DefaultShutdown() noexcept -> actor::Shutdown;
OPENTXS_EXPORT auto DefaultStartup() noexcept -> actor::StateMachine;
OPENTXS_EXPORT auto DefaultStateMachine() noexcept -> actor::Startup;
OPENTXS_EXPORT auto MakeArbitraryInproc() noexcept -> UnallocatedCString;
OPENTXS_EXPORT auto MakeArbitraryInproc(alloc::Default alloc) noexcept
    -> CString;
auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version) noexcept -> UnallocatedCString;
auto MakeDeterministicInproc(
    const std::string_view path,
    const int instance,
    const int version,
    const std::string_view suffix) noexcept -> UnallocatedCString;
auto RawToZ85(const ReadView input, Writer&& output) noexcept -> bool;
auto Z85ToRaw(
    const ReadView input,
    Writer&& output,
    bool inputIsNullTerminated = false) noexcept -> bool;
}  // namespace opentxs::network::zeromq
