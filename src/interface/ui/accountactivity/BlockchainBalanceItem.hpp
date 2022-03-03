// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include "1_Internal.hpp"
#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
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
    auto Type() const noexcept -> StorageBox final
    {
        return StorageBox::BLOCKCHAIN;
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
        const Identifier& accountID,
        const blockchain::Type chain,
        const OTData txid,
        const opentxs::Amount amount,
        const UnallocatedCString memo,
        const UnallocatedCString text) noexcept;
    ~BlockchainBalanceItem() final = default;

private:
    const blockchain::Type chain_;
    const OTData txid_;
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

    BlockchainBalanceItem() = delete;
    BlockchainBalanceItem(const BlockchainBalanceItem&) = delete;
    BlockchainBalanceItem(BlockchainBalanceItem&&) = delete;
    auto operator=(const BlockchainBalanceItem&)
        -> BlockchainBalanceItem& = delete;
    auto operator=(BlockchainBalanceItem&&) -> BlockchainBalanceItem& = delete;
};
}  // namespace opentxs::ui::implementation
