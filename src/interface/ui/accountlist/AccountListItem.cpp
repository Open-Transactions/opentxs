// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/AccountListItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto AccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>
{
    switch (ui::implementation::peek_custom<AccountType>(custom, 0)) {
        case AccountType::Blockchain: {

            return AccountListItemBlockchain(
                parent, api, rowID, sortKey, custom);
        }
        case AccountType::Custodial: {

            return AccountListItemCustodial(
                parent, api, rowID, sortKey, custom);
        }
        case AccountType::Error:
        default: {
            OT_FAIL;
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountListItem::AccountListItem(
    const AccountListInternalInterface& parent,
    const api::session::Client& api,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountListItemRow(parent, api, rowID, true)
    , api_(api)
    , type_(extract_custom<AccountType>(custom, 0))
    , unit_(std::get<0>(sortKey))
    , display_(display::GetDefinition(unit_))
    , unit_id_(extract_custom<identifier::UnitDefinition>(custom, 1))
    , notary_id_(extract_custom<identifier::Notary>(custom, 2))
    , unit_name_(display_.ShortName())
    , balance_(extract_custom<Amount>(custom, 3))
    , name_(std::get<1>(sortKey))
{
}

auto AccountListItem::Balance() const noexcept -> Amount
{
    auto lock = Lock{lock_};

    return balance_;
}

auto AccountListItem::DisplayBalance() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    if (auto out = display_.Format(balance_); false == out.empty()) {

        return out;
    }

    auto unformatted = UnallocatedCString{};
    balance_.Serialize(writer(unformatted));

    return unformatted;
}

auto AccountListItem::Name() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return name_;
}

auto AccountListItem::reindex(
    const AccountListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto& [unit, name] = key;
    const auto type = extract_custom<AccountType>(custom, 0);
    const auto contract = extract_custom<identifier::UnitDefinition>(custom, 1);
    const auto notary = extract_custom<identifier::Notary>(custom, 2);
    const auto balance = extract_custom<Amount>(custom, 3);

    OT_ASSERT(type_ == type);
    OT_ASSERT(unit_ == unit);
    OT_ASSERT(unit_id_ == contract);
    OT_ASSERT(notary_id_ == notary);

    auto lock = Lock{lock_};
    auto changed{false};

    if (balance_ != balance) {
        changed = true;
        balance_ = balance;
    }

    if (name_ != name) {
        changed = true;
        name_ = name;
    }

    return changed;
}

AccountListItem::~AccountListItem() = default;
}  // namespace opentxs::ui::implementation
