// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT ZMQQueue {
    using Message = ot::network::zeromq::Message;

    auto get(std::size_t index) noexcept(false) -> const Message&;
    auto receive(const Message& msg) noexcept -> void;

    ZMQQueue() noexcept;

    ~ZMQQueue();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
