// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/activitysummary/ActivitySummaryItem.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <ratio>
#include <thread>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/UniqueQueue.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

#define GET_TEXT_MILLISECONDS 10

namespace opentxs::factory
{
auto ActivitySummaryItem(
    const ui::implementation::ActivitySummaryInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivitySummaryRowID& rowID,
    const ui::implementation::ActivitySummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const Flag& running) noexcept
    -> std::shared_ptr<ui::implementation::ActivitySummaryRowInternal>
{
    using ReturnType = ui::implementation::ActivitySummaryItem;

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        running,
        ReturnType::LoadItemText(api, nymID, custom));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ActivitySummaryItem::ActivitySummaryItem(
    const ActivitySummaryInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ActivitySummaryRowID& rowID,
    const ActivitySummarySortKey& sortKey,
    CustomData& custom,
    const Flag& running,
    UnallocatedCString text) noexcept
    : ActivitySummaryItemRow(parent, api, rowID, true)
    , api_(api)
    , running_(running)
    , nym_id_(nymID)
    , key_(sortKey)
    , display_name_(std::get<1>(key_))
    , text_(text)
    , type_(extract_custom<otx::client::StorageBox>(custom, 1))
    , time_(extract_custom<Time>(custom, 3))
    , newest_item_thread_(nullptr)
    , newest_item_()
    , next_task_id_(0)
    , break_(false)
{
    startup(custom);
    newest_item_thread_ =
        std::make_unique<std::thread>(&ActivitySummaryItem::get_text, this);

    OT_ASSERT(newest_item_thread_);
}

auto ActivitySummaryItem::DisplayName() const noexcept -> UnallocatedCString
{
    sLock lock(shared_lock_);

    if (display_name_.empty()) { return api_.Contacts().ContactName(row_id_); }

    return display_name_;
}

auto ActivitySummaryItem::find_text(
    const PasswordPrompt& reason,
    const ItemLocator& locator) const noexcept -> UnallocatedCString
{
    const auto& [itemID, box, accountID, thread] = locator;

    switch (box) {
        case otx::client::StorageBox::MAILINBOX:
        case otx::client::StorageBox::MAILOUTBOX: {
            auto text = api_.Activity().MailText(
                nym_id_,
                api_.Factory().IdentifierFromBase58(itemID),
                box,
                reason);
            // TODO activity summary should subscribe for updates instead of
            // waiting for decryption

            return text.get();
        }
        case otx::client::StorageBox::INCOMINGCHEQUE:
        case otx::client::StorageBox::OUTGOINGCHEQUE: {
            auto text = api_.Activity().PaymentText(nym_id_, itemID, accountID);

            if (text) {

                return *text;
            } else {
                LogError()(OT_PRETTY_CLASS())("Cheque item does not exist.")
                    .Flush();
            }
        } break;
        case otx::client::StorageBox::BLOCKCHAIN: {
            return api_.Crypto().Blockchain().ActivityDescription(
                nym_id_, thread, itemID);
        }
        default: {
            OT_FAIL;
        }
    }

    return {};
}

void ActivitySummaryItem::get_text() noexcept
{
    auto reason = api_.Factory().PasswordPrompt(__func__);
    eLock lock(shared_lock_, std::defer_lock);
    auto locator = ItemLocator{"", {}, "", identifier::Generic{}};

    while (running_) {
        if (break_.load()) { return; }

        int taskID{0};

        if (newest_item_.Pop(taskID, locator)) {
            const auto text = find_text(reason, locator);
            lock.lock();
            text_ = text;
            lock.unlock();
            UpdateNotify();
        }

        Sleep(std::chrono::milliseconds(GET_TEXT_MILLISECONDS));
    }
}

auto ActivitySummaryItem::ImageURI() const noexcept -> UnallocatedCString
{
    // TODO

    return {};
}

auto ActivitySummaryItem::LoadItemText(
    const api::session::Client& api,
    const identifier::Nym& nym,
    const CustomData& custom) noexcept -> UnallocatedCString
{
    const auto& box =
        *static_cast<const otx::client::StorageBox*>(custom.at(1));
    const auto& thread = *static_cast<const identifier::Generic*>(custom.at(4));
    const auto& itemID = *static_cast<const UnallocatedCString*>(custom.at(0));

    if (otx::client::StorageBox::BLOCKCHAIN == box) {
        return api.Crypto().Blockchain().ActivityDescription(
            nym, thread, itemID);
    }

    return {};
}

auto ActivitySummaryItem::reindex(
    const ActivitySummarySortKey& key,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    key_ = key;
    lock.unlock();
    startup(custom);

    return true;
}

void ActivitySummaryItem::startup(CustomData& custom) noexcept
{
    auto locator = ItemLocator{
        extract_custom<UnallocatedCString>(custom, 0),
        type_,
        extract_custom<UnallocatedCString>(custom, 2),
        extract_custom<identifier::Generic>(custom, 4)};
    newest_item_.Push(++next_task_id_, std::move(locator));
}

auto ActivitySummaryItem::Text() const noexcept -> UnallocatedCString
{
    sLock lock(shared_lock_);

    return text_;
}

auto ActivitySummaryItem::ThreadID() const noexcept -> UnallocatedCString
{
    return row_id_.asBase58(api_.Crypto());
}

auto ActivitySummaryItem::Timestamp() const noexcept -> Time
{
    sLock lock(shared_lock_);

    return time_;
}

auto ActivitySummaryItem::Type() const noexcept -> otx::client::StorageBox
{
    sLock lock(shared_lock_);

    return type_;
}

ActivitySummaryItem::~ActivitySummaryItem()
{
    break_.store(true);

    if (newest_item_thread_ && newest_item_thread_->joinable()) {
        newest_item_thread_->join();
        newest_item_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation
