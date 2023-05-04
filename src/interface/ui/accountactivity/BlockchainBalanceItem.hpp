// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>

#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

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

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    auto Contacts() const noexcept
        -> UnallocatedVector<UnallocatedCString> final;
    auto DisplayAmount() const noexcept -> UnallocatedCString final;
    auto Memo() const noexcept -> UnallocatedCString final;
    auto Type() const noexcept -> otx::client::StorageBox final
    {
        return otx::client::StorageBox::BLOCKCHAIN;
    }
    auto UUID() const noexcept -> UnallocatedCString final;
    auto Workflow() const noexcept -> UnallocatedCString final { return {}; }

    BlockchainBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::session::Client& api,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        CustomData& custom,
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const blockchain::Type chain,
        const blockchain::block::TransactionHash& txid,
        const opentxs::Amount amount,
        const UnallocatedCString memo,
        const UnallocatedCString text) noexcept;
    BlockchainBalanceItem() = delete;
    BlockchainBalanceItem(const BlockchainBalanceItem&) = delete;
    BlockchainBalanceItem(BlockchainBalanceItem&&) = delete;
    auto operator=(const BlockchainBalanceItem&)
        -> BlockchainBalanceItem& = delete;
    auto operator=(BlockchainBalanceItem&&) -> BlockchainBalanceItem& = delete;

    ~BlockchainBalanceItem() final = default;

private:
    const blockchain::Type chain_;
    const blockchain::block::TransactionHash txid_;
    opentxs::Amount amount_;
    UnallocatedCString memo_;
    std::atomic_int confirmations_;

    auto effective_amount() const noexcept -> opentxs::Amount final
    {
        sLock lock(shared_lock_);
        return amount_;
    }

    auto reindex(
        const implementation::AccountActivitySortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation
