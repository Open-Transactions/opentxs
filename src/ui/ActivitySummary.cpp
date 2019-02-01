// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"

#include "ActivitySummaryItemBlank.hpp"
#include "InternalUI.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <thread>
#include <tuple>
#include <vector>

#include "ActivitySummary.hpp"

#define OT_METHOD "opentxs::ui::implementation::ActivitySummary::"

namespace opentxs
{
ui::implementation::ActivitySummaryExternalInterface* Factory::ActivitySummary(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Flag& running,
    const identifier::Nym& nymID)
{
    return new ui::implementation::ActivitySummary(
        api, publisher, running, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{

ActivitySummary::ActivitySummary(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Flag& running,
    const identifier::Nym& nymID)
    : ActivitySummaryList(api, publisher, nymID)
    , listeners_{{api_.Activity().ThreadPublisher(nymID),
        new MessageProcessor<ActivitySummary>(
            &ActivitySummary::process_thread)},
}
    , running_(running)
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&ActivitySummary::startup, this));

    OT_ASSERT(startup_)
}

void ActivitySummary::construct_row(
    const ActivitySummaryRowID& id,
    const ActivitySummarySortKey& index,
    const CustomData& custom) const
{
    items_[index].emplace(
        id,
        Factory::ActivitySummaryItem(
            *this, api_, publisher_, nym_id_, id, index, custom, running_));
    names_.emplace(id, index);
}

std::string ActivitySummary::display_name(
    const proto::StorageThread& thread) const
{
    std::set<std::string> names{};

    for (const auto& participant : thread.participant()) {
        auto name =
            api_.Contacts().ContactName(Identifier::Factory(participant));

        if (name.empty()) {
            names.emplace(participant);
        } else {
            names.emplace(std::move(name));
        }
    }

    if (names.empty()) { return thread.id(); }

    std::stringstream stream{};

    for (const auto& name : names) { stream << name << ", "; }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 2, 2); }

    return output;
}

const proto::StorageThreadItem& ActivitySummary::newest_item(
    const proto::StorageThread& thread,
    CustomData& custom)
{
    const proto::StorageThreadItem* output{nullptr};

    for (const auto& item : thread.item()) {
        if (nullptr == output) {
            output = &item;

            continue;
        }

        if (item.time() > output->time()) {
            output = &item;

            continue;
        }
    }

    OT_ASSERT(nullptr != output)

    custom.emplace_back(new std::string(output->id()));
    custom.emplace_back(new StorageBox(static_cast<StorageBox>(output->box())));
    custom.emplace_back(new std::string(output->account()));

    return *output;
}

void ActivitySummary::process_thread(const std::string& id)
{
    const auto threadID = Identifier::Factory(id);
    const auto thread = api_.Activity().Thread(nym_id_, threadID);

    OT_ASSERT(thread);

    CustomData custom{};
    const auto name = display_name(*thread);
    const auto time = std::chrono::system_clock::time_point(
        std::chrono::seconds(newest_item(*thread, custom).time()));
    const ActivitySummarySortKey index{time, name};
    add_item(threadID, index, custom);
}

void ActivitySummary::process_thread(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto threadID = Identifier::Factory(id);

    OT_ASSERT(false == threadID->empty())

    auto existing = names_.count(threadID);

    if (0 == existing) { process_thread(id); }
}

void ActivitySummary::startup()
{
    const auto threads = api_.Activity().Threads(nym_id_, false);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(threads.size())(
        " threads.")
        .Flush();
    for (const auto& [id, alias] : threads) {
        [[maybe_unused]] const auto& notUsed = alias;
        process_thread(id);
    }

    startup_complete_->On();
}

ActivitySummary::~ActivitySummary()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
