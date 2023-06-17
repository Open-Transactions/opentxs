// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>

#include "internal/blockchain/node/wallet/Types.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"

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

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class BitcoinTransactionBuilder
{
public:
    auto operator()() noexcept -> BuildResult;

    BitcoinTransactionBuilder(
        const api::Session& api,
        const node::Manager& node,
        const identifier::Generic& id,
        const Type chain,
        database::Wallet& db,
        proto::BlockchainTransactionProposal& proposal,
        std::promise<SendOutcome>& promise) noexcept;
    BitcoinTransactionBuilder() = delete;

    ~BitcoinTransactionBuilder();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
