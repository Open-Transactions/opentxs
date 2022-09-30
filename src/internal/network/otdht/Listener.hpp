// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <string_view>

#include "internal/network/otdht/Node.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class Listener
{
public:
    class Actor;

    static auto NextID(alloc::Default alloc) noexcept -> CString;

    auto Init() noexcept -> void;

    Listener(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Node::Shared> shared,
        std::string_view routerBind,
        std::string_view routerAdvertise,
        std::string_view publishBind,
        std::string_view publishAdvertise,
        std::string_view routingID,
        std::string_view fromNode) noexcept;
    Listener() = delete;
    Listener(const Listener&) = delete;
    Listener(Listener&&) = delete;
    auto operator=(const Listener&) -> Listener& = delete;
    auto operator=(Listener&&) -> Listener& = delete;

    ~Listener();

private:
    boost::shared_ptr<Actor> actor_;
};
}  // namespace opentxs::network::otdht
