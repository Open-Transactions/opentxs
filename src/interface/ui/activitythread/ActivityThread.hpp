// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/ActivityThread.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Blank.hpp"
#include "util/Work.hpp"

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

class Session;
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

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class StorageThread;
class StorageThreadItem;
}  // namespace proto

class Contact;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
using STORAGEID = std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

template <>
struct less<STORAGEID> {
    auto operator()(const STORAGEID& lhs, const STORAGEID& rhs) const -> bool
    {
        const auto& [lID, lBox, lAccount] = lhs;
        const auto& [rID, rBox, rAccount] = rhs;

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lBox < rBox) { return true; }

        if (rBox < lBox) { return false; }

        if (lAccount->str() < rAccount->str()) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs
{
template <>
struct make_blank<ui::implementation::ActivityThreadRowID> {
    static auto value(const api::Session& api)
        -> ui::implementation::ActivityThreadRowID
    {
        return {api.Factory().Identifier(), {}, api.Factory().Identifier()};
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
    auto CanMessage() const noexcept -> bool final;
    auto ClearCallbacks() const noexcept -> void final;
    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto GetDraft() const noexcept -> UnallocatedCString final;
    auto Participants() const noexcept -> UnallocatedCString final;
    auto Pay(
        const UnallocatedCString& amount,
        const Identifier& sourceAccount,
        const UnallocatedCString& memo,
        const PaymentType type) const noexcept -> bool final;
    auto Pay(
        const Amount amount,
        const Identifier& sourceAccount,
        const UnallocatedCString& memo,
        const PaymentType type) const noexcept -> bool final;
    auto PaymentCode(const UnitType currency) const noexcept
        -> UnallocatedCString final;
    auto SendDraft() const noexcept -> bool final;
    auto SetDraft(const UnallocatedCString& draft) const noexcept -> bool final;
    auto ThreadID() const noexcept -> UnallocatedCString final;

    auto SetCallbacks(Callbacks&&) noexcept -> void final;

    ActivityThread(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback& cb) noexcept;

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

    const OTIdentifier threadID_;
    const OTIdentifier self_contact_;
    const UnallocatedSet<OTIdentifier> contacts_;
    const UnallocatedCString participants_;
    UnallocatedCString me_;
    UnallocatedCString display_name_;
    UnallocatedMap<UnitType, UnallocatedCString> payment_codes_;
    std::optional<Messagability> can_message_;
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
        const Identifier& sourceAccount,
        const UnallocatedCString& memo) const noexcept -> bool;
    auto validate_account(const Identifier& sourceAccount) const noexcept
        -> bool;

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
    auto update_messagability(Messagability value) noexcept -> bool;
    auto update_payment_codes() noexcept -> bool;

    ActivityThread() = delete;
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    auto operator=(const ActivityThread&) -> ActivityThread& = delete;
    auto operator=(ActivityThread&&) -> ActivityThread& = delete;
};
}  // namespace opentxs::ui::implementation
