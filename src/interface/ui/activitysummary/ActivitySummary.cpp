// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/activitysummary/ActivitySummary.hpp"  // IWYU pragma: associated

#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <chrono>
#include <memory>
#include <sstream>
#include <thread>
#include <utility>

#include "Proto.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::factory
{
auto ActivitySummaryModel(
    const api::session::Client& api,
    const Flag& running,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ActivitySummary>
{
    using ReturnType = ui::implementation::ActivitySummary;

    return std::make_unique<ReturnType>(api, running, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ActivitySummary::ActivitySummary(
    const api::session::Client& api,
    const Flag& running,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : ActivitySummaryList(api, nymID, cb, true)
    , api_(api)
    , listeners_(
          {{api.Activity().ThreadPublisher(nymID),
            new MessageProcessor<ActivitySummary>(
                &ActivitySummary::process_thread)}})
    , running_(running)
{
    setup_listeners(api_, listeners_);
    startup_ = std::make_unique<std::thread>(&ActivitySummary::startup, this);

    OT_ASSERT(startup_);
}

auto ActivitySummary::construct_row(
    const ActivitySummaryRowID& id,
    const ActivitySummarySortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ActivitySummaryItem(
        *this, api_, primary_id_, id, index, custom, running_);
}

auto ActivitySummary::display_name(
    const proto::StorageThread& thread) const noexcept -> UnallocatedCString
{
    auto names = UnallocatedSet<UnallocatedCString>{};

    for (const auto& id : thread.participant()) {
        names.emplace(api_.Contacts().ContactName(
            api_.Factory().IdentifierFromBase58(id)));
    }

    if (names.empty()) { return thread.id(); }

    std::stringstream stream{};

    for (const auto& name : names) { stream << name << ", "; }

    UnallocatedCString output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 2, 2); }

    return output;
}

auto ActivitySummary::newest_item(
    const identifier::Generic& id,
    const proto::StorageThread& thread,
    CustomData& custom) noexcept -> const proto::StorageThreadItem&
{
    const proto::StorageThreadItem* output{nullptr};
    auto* time = new Time;

    OT_ASSERT(nullptr != time);

    for (const auto& item : thread.item()) {
        if (nullptr == output) {
            output = &item;
            *time = convert_stime(item.time());

            continue;
        }

        if (item.time() > output->time()) {
            output = &item;
            *time = convert_stime(item.time());

            continue;
        }
    }

    OT_ASSERT(nullptr != output);

    custom.emplace_back(new UnallocatedCString(output->id()));
    custom.emplace_back(new otx::client::StorageBox(
        static_cast<otx::client::StorageBox>(output->box())));
    custom.emplace_back(new UnallocatedCString(output->account()));
    custom.emplace_back(time);
    custom.emplace_back(new identifier::Generic{id});

    return *output;
}

void ActivitySummary::process_thread(const UnallocatedCString& id) noexcept
{
    const auto threadID = api_.Factory().IdentifierFromBase58(id);
    auto thread = proto::StorageThread{};
    auto loaded = api_.Activity().Thread(primary_id_, threadID, thread);

    OT_ASSERT(loaded);

    auto custom = CustomData{};
    const auto name = display_name(thread);
    const auto time = Time(
        std::chrono::seconds(newest_item(threadID, thread, custom).time()));
    const ActivitySummarySortKey index{time, name};
    add_item(threadID, index, custom);
}

void ActivitySummary::process_thread(const Message& message) noexcept
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto threadID = api_.Factory().IdentifierFromHash(body.at(1).Bytes());

    OT_ASSERT(false == threadID.empty());

    delete_item(threadID);
    process_thread(threadID.asBase58(api_.Crypto()));
}

void ActivitySummary::startup() noexcept
{
    const auto threads = api_.Activity().Threads(primary_id_, false);
    LogDetail()(OT_PRETTY_CLASS())("Loading ")(threads.size())(" threads.")
        .Flush();
    for (const auto& [id, alias] : threads) {
        [[maybe_unused]] const auto& notUsed = alias;
        process_thread(id);
    }

    finish_startup();
}

ActivitySummary::~ActivitySummary()
{
    for (const auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
