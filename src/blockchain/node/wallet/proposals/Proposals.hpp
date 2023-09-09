// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
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
namespace wallet
{
class ProposalsPrivate;
}  // namespace wallet

class Manager;
class Spend;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Proposals
{
public:
    auto Add(const node::Spend& spend, std::promise<SendOutcome>&& promise)
        const noexcept -> bool;

    auto Run() noexcept -> bool;

    Proposals(
        const api::session::Client& api,
        const node::Manager& node,
        database::Wallet& db,
        const Type chain) noexcept;
    ~Proposals();

private:
    std::unique_ptr<ProposalsPrivate> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
