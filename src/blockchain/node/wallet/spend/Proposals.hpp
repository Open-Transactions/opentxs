// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainTransactionProposal.pb.h>
#include <future>
#include <memory>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
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
class Proposals
{
public:
    using Proposal = proto::BlockchainTransactionProposal;

    auto Add(const Proposal& tx, std::promise<SendOutcome>&& promise)
        const noexcept -> void;

    auto Run() noexcept -> bool;

    Proposals(
        const api::Session& api,
        const node::Manager& node,
        database::Wallet& db,
        const Type chain) noexcept;
    ~Proposals();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
