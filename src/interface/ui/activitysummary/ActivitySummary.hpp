// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class StorageThread;
class StorageThreadItem;
}  // namespace proto

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ActivitySummaryList = List<
    ActivitySummaryExternalInterface,
    ActivitySummaryInternalInterface,
    ActivitySummaryRowID,
    ActivitySummaryRowInterface,
    ActivitySummaryRowInternal,
    ActivitySummaryRowBlank,
    ActivitySummarySortKey,
    ActivitySummaryPrimaryID>;

class ActivitySummary final : public ActivitySummaryList
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }

    ActivitySummary(
        const api::session::Client& api,
        const Flag& running,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    ActivitySummary() = delete;
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    auto operator=(const ActivitySummary&) -> ActivitySummary& = delete;
    auto operator=(ActivitySummary&&) -> ActivitySummary& = delete;

    ~ActivitySummary() final;

private:
    const ListenerDefinitions listeners_;
    const Flag& running_;

    static auto newest_item(
        const identifier::Generic& id,
        const proto::StorageThread& thread,
        CustomData& custom) noexcept -> const proto::StorageThreadItem&;

    auto construct_row(
        const ActivitySummaryRowID& id,
        const ActivitySummarySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto display_name(const proto::StorageThread& thread) const noexcept
        -> UnallocatedCString;

    void process_thread(const UnallocatedCString& threadID) noexcept;
    void process_thread(const Message& message) noexcept;
    void startup() noexcept;
};
}  // namespace opentxs::ui::implementation
