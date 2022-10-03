// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <tuple>

#include "interface/ui/activitythread/ActivityThreadItem.hpp"
#include "internal/interface/ui/UI.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class PendingSend final : public ActivityThreadItem
{
public:
    static auto extract(CustomData& custom) noexcept
        -> std::tuple<opentxs::Amount, UnallocatedCString, UnallocatedCString>;

    auto Amount() const noexcept -> opentxs::Amount final;
    auto Deposit() const noexcept -> bool final { return false; }
    auto DisplayAmount() const noexcept -> UnallocatedCString final;
    auto Memo() const noexcept -> UnallocatedCString final;

    PendingSend(
        const ActivityThreadInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        opentxs::Amount amount,
        UnallocatedCString&& display,
        UnallocatedCString&& memo) noexcept;
    PendingSend() = delete;
    PendingSend(const PendingSend&) = delete;
    PendingSend(PendingSend&&) = delete;
    auto operator=(const PendingSend&) -> PendingSend& = delete;
    auto operator=(PendingSend&&) -> PendingSend& = delete;

    ~PendingSend() final = default;

private:
    opentxs::Amount amount_;
    UnallocatedCString display_amount_;
    UnallocatedCString memo_;

    auto reindex(const ActivityThreadSortKey& key, CustomData& custom) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation
