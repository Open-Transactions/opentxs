// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accounttree/AccountTreeItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <tuple>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/AccountType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto AccountTreeItem(
    const ui::implementation::AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountCurrencyRowID& rowID,
    const ui::implementation::AccountCurrencySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountCurrencyRowInternal>
{
    switch (std::get<1>(sortKey)) {
        case AccountType::Blockchain: {

            return AccountTreeItemBlockchain(
                parent, api, rowID, sortKey, custom);
        }
        case AccountType::Custodial: {

            return AccountTreeItemCustodial(
                parent, api, rowID, sortKey, custom);
        }
        case AccountType::Error:
        default: {
            LogAbort()(__FUNCTION__)(": invalid account type").Abort();
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountTreeItem::AccountTreeItem(
    const AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const AccountCurrencyRowID& rowID,
    const AccountCurrencySortKey& sortKey,
    CustomData& custom) noexcept
    : AccountTreeItemRow(parent, api, rowID, true)
    , api_(api)
    , type_(std::get<1>(sortKey))
    , unit_(extract_custom<UnitType>(custom, 0))
    , display_(display::GetDefinition(unit_))
    , unit_id_(extract_custom<identifier::UnitDefinition>(custom, 1))
    , notary_id_(extract_custom<identifier::Notary>(custom, 2))
    , unit_name_(display_.ShortName())
    , balance_(extract_custom<Amount>(custom, 3))
    , name_(std::get<2>(sortKey))
{
}

auto AccountTreeItem::Balance() const noexcept -> Amount
{
    auto lock = Lock{lock_};

    return balance_;
}

auto AccountTreeItem::DisplayBalance() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    if (auto out = display_.Format(balance_); false == out.empty()) {

        return out;
    }

    auto unformatted = UnallocatedCString{};
    balance_.Serialize(writer(unformatted));

    return unformatted;
}

auto AccountTreeItem::Name() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return name_;
}

auto AccountTreeItem::reindex(
    const AccountCurrencySortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto& [index, type, name] = key;

    const auto unit = extract_custom<UnitType>(custom, 0);
    const auto contract = extract_custom<identifier::UnitDefinition>(custom, 1);
    const auto notary = extract_custom<identifier::Notary>(custom, 2);
    const auto balance = extract_custom<Amount>(custom, 3);

    assert_true(type_ == type);
    assert_true(unit_ == unit);
    assert_true(unit_id_ == contract);
    assert_true(notary_id_ == notary);

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

AccountTreeItem::~AccountTreeItem() = default;
}  // namespace opentxs::ui::implementation
