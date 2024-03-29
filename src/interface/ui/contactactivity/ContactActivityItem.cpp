// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contactactivity/ContactActivityItem.hpp"  // IWYU pragma: associated

#include <tuple>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::implementation
{
ContactActivityItem::ContactActivityItem(
    const ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ContactActivityRowID& rowID,
    const ContactActivitySortKey& sortKey,
    CustomData& custom) noexcept
    : ContactActivityItemRow(parent, api, rowID, true)
    , api_(api)
    , nym_id_(nymID)
    , time_(std::get<0>(sortKey))
    , item_id_(std::get<0>(row_id_))
    , box_(std::get<1>(row_id_))
    , account_id_(std::get<2>(row_id_))
    , from_(extract_custom<UnallocatedCString>(custom, 0))
    , text_(extract_custom<UnallocatedCString>(custom, 1))
    , loading_(Flag::Factory(extract_custom<bool>(custom, 2)))
    , pending_(Flag::Factory(extract_custom<bool>(custom, 3)))
    , outgoing_(Flag::Factory(extract_custom<bool>(custom, 4)))
{
    assert_true(verify_empty(custom));
}

auto ContactActivityItem::From() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return from_;
}

auto ContactActivityItem::MarkRead() const noexcept -> bool
{
    return api_.Activity().MarkRead(
        nym_id_,
        api_.Factory().IdentifierFromBase58(parent_.ThreadID()),
        item_id_);
}

auto ContactActivityItem::reindex(
    const ContactActivitySortKey&,
    CustomData& custom) noexcept -> bool
{
    const auto from = extract_custom<UnallocatedCString>(custom, 0);
    const auto text = extract_custom<UnallocatedCString>(custom, 1);
    auto changed{false};

    {
        auto lock = eLock{shared_lock_};

        if (text_ != text) {
            text_ = text;
            changed = true;
        }

        if (from_ != from) {
            from_ = from;
            changed = true;
        }
    }

    const auto loading = extract_custom<bool>(custom, 2);
    const auto pending = extract_custom<bool>(custom, 3);
    const auto outgoing = extract_custom<bool>(custom, 4);

    if (const auto val = loading_->Set(loading); val != loading) {
        changed = true;
    }

    if (const auto val = pending_->Set(pending); val != pending) {
        changed = true;
    }

    if (const auto val = outgoing_->Set(outgoing); val != outgoing) {
        changed = true;
    }

    return changed;
}

auto ContactActivityItem::Text() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return text_;
}

auto ContactActivityItem::Timestamp() const noexcept -> Time
{
    auto lock = sLock{shared_lock_};

    return time_;
}
}  // namespace opentxs::ui::implementation
