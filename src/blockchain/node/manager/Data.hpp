// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/network/zeromq/socket/Raw.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
struct Endpoints;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::manager
{
class Data
{
public:
    network::zeromq::socket::Raw to_actor_;
    network::zeromq::socket::Raw to_peer_manager_;
    network::zeromq::socket::Raw to_dht_;
    std::weak_ptr<node::Manager> self_;

    Data(const api::Session& api, const node::Endpoints& endpoints) noexcept;

    ~Data();
};
}  // namespace opentxs::blockchain::node::manager
