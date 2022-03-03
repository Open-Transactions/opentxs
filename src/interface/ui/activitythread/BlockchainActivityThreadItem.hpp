// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <thread>
#include <tuple>

#include "1_Internal.hpp"
#include "interface/ui/activitythread/ActivityThreadItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class BlockchainActivityThreadItem final : public ActivityThreadItem
{
public:
    static auto extract(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        CustomData& custom) noexcept -> std::
        tuple<OTData, opentxs::Amount, UnallocatedCString, UnallocatedCString>;

    auto Amount() const noexcept -> opentxs::Amount final;
    auto DisplayAmount() const noexcept -> UnallocatedCString final;
    auto Memo() const noexcept -> UnallocatedCString final;

    BlockchainActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        OTData&& txid,
        opentxs::Amount amount,
        UnallocatedCString&& displayAmount,
        UnallocatedCString&& memo) noexcept;

    ~BlockchainActivityThreadItem() final = default;

private:
    const OTData txid_;
    UnallocatedCString display_amount_;
    UnallocatedCString memo_;
    opentxs::Amount amount_;

    auto reindex(const ActivityThreadSortKey& key, CustomData& custom) noexcept
        -> bool final;

    BlockchainActivityThreadItem() = delete;
    BlockchainActivityThreadItem(const BlockchainActivityThreadItem&) = delete;
    BlockchainActivityThreadItem(BlockchainActivityThreadItem&&) = delete;
    auto operator=(const BlockchainActivityThreadItem&)
        -> BlockchainActivityThreadItem& = delete;
    auto operator=(BlockchainActivityThreadItem&&)
        -> BlockchainActivityThreadItem& = delete;
};
}  // namespace opentxs::ui::implementation
