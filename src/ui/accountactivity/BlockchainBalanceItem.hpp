// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "ui/accountactivity/BalanceItem.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class BlockchainBalanceItem final : public BalanceItem
{
public:
    auto Amount() const noexcept -> opentxs::Amount final
    {
        return effective_amount();
    }
    auto Confirmations() const noexcept -> int final { return confirmations_; }
    auto Contacts() const noexcept -> std::pmr::vector<std::string> final;
    auto DisplayAmount() const noexcept -> std::string final;
    auto Memo() const noexcept -> std::string final;
    auto Type() const noexcept -> StorageBox final
    {
        return StorageBox::BLOCKCHAIN;
    }
    auto UUID() const noexcept -> std::string final;
    auto Workflow() const noexcept -> std::string final { return {}; }

    BlockchainBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::session::Client& api,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Type chain,
        const OTData txid,
        const opentxs::Amount amount,
        const std::string memo,
        const std::string text) noexcept;
    ~BlockchainBalanceItem() final = default;

private:
    const blockchain::Type chain_;
    const OTData txid_;
    opentxs::Amount amount_;
    std::string memo_;
    std::atomic_int confirmations_;

    auto effective_amount() const noexcept -> opentxs::Amount final
    {
        sLock lock(shared_lock_);
        return amount_;
    }

    auto reindex(
        const implementation::AccountActivitySortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;

    BlockchainBalanceItem() = delete;
    BlockchainBalanceItem(const BlockchainBalanceItem&) = delete;
    BlockchainBalanceItem(BlockchainBalanceItem&&) = delete;
    auto operator=(const BlockchainBalanceItem&)
        -> BlockchainBalanceItem& = delete;
    auto operator=(BlockchainBalanceItem&&) -> BlockchainBalanceItem& = delete;
};
}  // namespace opentxs::ui::implementation
