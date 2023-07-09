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
class Peer
{
public:
    class Actor;

    static auto NextID(alloc::Strategy alloc) noexcept -> CString;

    auto Init() noexcept -> void;

    Peer(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Node::Shared> shared,
        std::string_view routingID,
        std::string_view toRemote,
        std::string_view fromNode) noexcept;
    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;

    ~Peer();

private:
    boost::shared_ptr<Actor> actor_;
};
}  // namespace opentxs::network::otdht
