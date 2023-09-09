// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <functional>
#include <future>

#include "blockchain/node/wallet/proposals/Pending.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

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
class Manager;
class Spend;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class ProposalsPrivate
{
public:
    auto Add(const node::Spend& spend, std::promise<SendOutcome>&& promise)
        const noexcept -> bool;
    auto Run() noexcept -> bool;

    ProposalsPrivate(
        const api::session::Client& api,
        const node::Manager& node,
        database::Wallet& db,
        const Type chain) noexcept;

private:
    using Builder = std::function<BuildResult(
        const identifier::Generic& id,
        node::Spend&,
        std::promise<SendOutcome>&)>;

    struct Data {
        database::Wallet& db_;
        Pending pending_;
        UnallocatedMap<identifier::Generic, Time> confirming_;

        Data(database::Wallet& db) noexcept
            : db_(db)
            , pending_()
            , confirming_()
        {
        }
    };

    using Guarded = libguarded::plain_guarded<Data>;

    const api::session::Client& api_;
    const node::Manager& node_;
    const Type chain_;
    mutable Guarded data_;

    auto build_transaction_bitcoin(
        Data& data,
        const identifier::Generic& id,
        node::Spend& proposal,
        std::promise<SendOutcome>& promise) const noexcept -> BuildResult;
    auto cleanup(Data& data) noexcept -> void;
    auto get_builder(Data& data) const noexcept -> Builder;
    auto rebroadcast(Data& data) noexcept -> void;
    auto send(Data& data) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet
