// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <tuple>

#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Amount.hpp"
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
class BlockchainContactActivityItem final : public ContactActivityItem
{
public:
    static auto extract(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        CustomData& custom) noexcept
        -> std::tuple<
            blockchain::block::TransactionHash,
            opentxs::Amount,
            UnallocatedCString,
            UnallocatedCString>;

    auto Amount() const noexcept -> opentxs::Amount final;
    auto DisplayAmount() const noexcept -> UnallocatedCString final;
    auto Memo() const noexcept -> UnallocatedCString final;
    auto TXID() const noexcept -> UnallocatedCString final;

    BlockchainContactActivityItem(
        const ContactActivityInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ContactActivityRowID& rowID,
        const ContactActivitySortKey& sortKey,
        CustomData& custom,
        blockchain::block::TransactionHash&& txid,
        opentxs::Amount amount,
        UnallocatedCString&& displayAmount,
        UnallocatedCString&& memo) noexcept;
    BlockchainContactActivityItem() = delete;
    BlockchainContactActivityItem(const BlockchainContactActivityItem&) =
        delete;
    BlockchainContactActivityItem(BlockchainContactActivityItem&&) = delete;
    auto operator=(const BlockchainContactActivityItem&)
        -> BlockchainContactActivityItem& = delete;
    auto operator=(BlockchainContactActivityItem&&)
        -> BlockchainContactActivityItem& = delete;

    ~BlockchainContactActivityItem() final = default;

private:
    const blockchain::block::TransactionHash txid_;
    UnallocatedCString display_amount_;
    UnallocatedCString memo_;
    opentxs::Amount amount_;

    auto reindex(const ContactActivitySortKey& key, CustomData& custom) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation
