// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>

#include "internal/blockchain/node/wallet/Types.hpp"
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
namespace wallet
{
class BitcoinTransactionBuilderPrivate;
}  // namespace wallet

class Manager;
class Spend;
}  // namespace node
}  // namespace blockchain
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
        node::Spend& proposal,
        std::promise<SendOutcome>& promise) noexcept;
    BitcoinTransactionBuilder() = delete;

    ~BitcoinTransactionBuilder();

private:
    std::unique_ptr<BitcoinTransactionBuilderPrivate> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
