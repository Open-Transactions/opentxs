// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <utility>

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Blank.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class StorageThread;
class StorageThreadItem;
}  // namespace proto

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <>
struct make_blank<ui::implementation::ActivityThreadRowID> {
    static auto value(const api::Session& api)
        -> ui::implementation::ActivityThreadRowID
    {
        return {identifier::Generic{}, {}, identifier::Generic{}};
    }
};

using DraftTask = std::pair<
    ui::implementation::ActivityThreadRowID,
    api::session::OTX::BackgroundTask>;

template <>
struct make_blank<DraftTask> {
    static auto value(const api::Session& api) -> DraftTask
    {
        return {
            make_blank<ui::implementation::ActivityThreadRowID>::value(api),
            make_blank<api::session::OTX::BackgroundTask>::value(api)};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ActivityThreadList = List<
    ActivityThreadExternalInterface,
    ActivityThreadInternalInterface,
    ActivityThreadRowID,
    ActivityThreadRowInterface,
    ActivityThreadRowInternal,
    ActivityThreadRowBlank,
    ActivityThreadSortKey,
    ActivityThreadPrimaryID>;

class ActivityThread final : public ActivityThreadList, Worker<ActivityThread>
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto CanMessage() const noexcept -> bool final;
    auto ClearCallbacks() const noexcept -> void final;
    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto GetDraft() const noexcept -> UnallocatedCString final;
    auto Participants() const noexcept -> UnallocatedCString final;
    auto Pay(
        const UnallocatedCString& amount,
        const identifier::Generic& sourceAccount,
        const UnallocatedCString& memo,
        const otx::client::PaymentType type) const noexcept -> bool final;
    auto Pay(
        const Amount amount,
        const identifier::Generic& sourceAccount,
        const UnallocatedCString& memo,
        const otx::client::PaymentType type) const noexcept -> bool final;
    auto PaymentCode(const UnitType currency) const noexcept
        -> UnallocatedCString final;
    auto SendDraft() const noexcept -> bool final;
    auto SetDraft(const UnallocatedCString& draft) const noexcept -> bool final;
    auto ThreadID() const noexcept -> UnallocatedCString final;

    auto SetCallbacks(Callbacks&&) noexcept -> void final;

    ActivityThread(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback& cb) noexcept;
    ActivityThread() = delete;
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    auto operator=(const ActivityThread&) -> ActivityThread& = delete;
    auto operator=(ActivityThread&&) -> ActivityThread& = delete;

    ~ActivityThread() final;

private:
    friend Worker<ActivityThread>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        contact = value(WorkType::ContactUpdated),
        thread = value(WorkType::ActivityThreadUpdated),
        message_loaded = value(WorkType::MessageLoaded),
        otx = value(WorkType::OTXTaskComplete),
        messagability = value(WorkType::OTXMessagability),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const identifier::Generic thread_id_;
    const identifier::Generic self_contact_;
    const UnallocatedSet<identifier::Generic> contacts_;
    const UnallocatedCString participants_;
    UnallocatedCString me_;
    UnallocatedCString display_name_;
    UnallocatedMap<UnitType, UnallocatedCString> payment_codes_;
    std::optional<otx::client::Messagability> can_message_;
    mutable UnallocatedCString draft_;
    mutable UnallocatedMap<api::session::OTX::TaskID, DraftTask> draft_tasks_;
    mutable std::optional<Callbacks> callbacks_;

    auto calculate_display_name() const noexcept -> UnallocatedCString;
    auto calculate_participants() const noexcept -> UnallocatedCString;
    auto comma(const UnallocatedSet<UnallocatedCString>& list) const noexcept
        -> UnallocatedCString;
    auto can_message() const noexcept -> bool;
    auto construct_row(
        const ActivityThreadRowID& id,
        const ActivityThreadSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto from(bool outgoing) const noexcept -> UnallocatedCString;
    auto send_cheque(
        const Amount amount,
        const identifier::Generic& sourceAccount,
        const UnallocatedCString& memo) const noexcept -> bool;
    auto validate_account(
        const identifier::Generic& sourceAccount) const noexcept -> bool;

    auto load_contacts(const proto::StorageThread& thread) noexcept -> void;
    auto load_thread(const proto::StorageThread& thread) noexcept -> void;
    auto new_thread() noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto process_item(const proto::StorageThreadItem& item) noexcept(false)
        -> ActivityThreadRowID;
    auto process_messagability(const Message& message) noexcept -> void;
    auto process_message_loaded(const Message& message) noexcept -> void;
    auto process_otx(const Message& message) noexcept -> void;
    auto process_thread(const Message& message) noexcept -> void;
    auto refresh_thread() noexcept -> void;
    auto set_participants() noexcept -> void;
    auto state_machine() noexcept -> bool final;
    auto startup() noexcept -> void;
    auto update_display_name() noexcept -> bool;
    auto update_messagability(otx::client::Messagability value) noexcept
        -> bool;
    auto update_payment_codes() noexcept -> bool;
};
}  // namespace opentxs::ui::implementation
