// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.hpp"  // IWYU pragma: keep
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using MessagableListList = List<
    MessagableExternalInterface,
    MessagableInternalInterface,
    MessagableListRowID,
    MessagableListRowInterface,
    MessagableListRowInternal,
    MessagableListRowBlank,
    MessagableListSortKey,
    MessagableListPrimaryID>;

class MessagableList final : public MessagableListList, Worker<MessagableList>
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ID() const noexcept -> const identifier::Generic& final
    {
        return owner_contact_id_;
    }

    MessagableList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    MessagableList() = delete;
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    auto operator=(const MessagableList&) -> MessagableList& = delete;
    auto operator=(MessagableList&&) -> MessagableList& = delete;

    ~MessagableList() final;

private:
    friend Worker<MessagableList>;

    enum class Work : OTZMQWorkType {
        contact = value(WorkType::ContactUpdated),
        nym = value(WorkType::NymUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    const identifier::Generic owner_contact_id_;

    auto construct_row(
        const MessagableListRowID& id,
        const MessagableListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto last(const MessagableListRowID& id) const noexcept -> bool final
    {
        return MessagableListList::last(id);
    }

    auto pipeline(const Message& in) noexcept -> void;
    auto process_contact(
        const MessagableListRowID& id,
        const MessagableListSortKey& key) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto process_nym(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
