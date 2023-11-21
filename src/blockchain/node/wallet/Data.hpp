// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/node/wallet/proposals/Proposals.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Client;
}  // namespace internal
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace database
{
class Wallet;
}  // namespace database

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Data
{
public:
    network::zeromq::socket::Raw to_actor_;
    Proposals proposals_;

    Data(
        std::shared_ptr<const api::session::internal::Client> api,
        std::shared_ptr<const node::Manager> node,
        database::Wallet& db) noexcept;

    ~Data();
};
}  // namespace opentxs::blockchain::node::wallet
