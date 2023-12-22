// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountsummary/IssuerItem.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <thread>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto IssuerItem(
    const ui::implementation::AccountSummaryInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountSummaryRowID& rowID,
    const ui::implementation::AccountSummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const UnitType currency) noexcept
    -> std::shared_ptr<ui::implementation::AccountSummaryRowInternal>
{
    using ReturnType = ui::implementation::IssuerItem;

    return std::make_shared<ReturnType>(
        parent, api, rowID, sortKey, custom, currency);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
IssuerItem::IssuerItem(
    const AccountSummaryInternalInterface& parent,
    const api::session::Client& api,
    const AccountSummaryRowID& rowID,
    const AccountSummarySortKey& key,
    [[maybe_unused]] CustomData& custom,
    const UnitType currency) noexcept
    : Combined(
          api,
          parent.NymID(),
          parent.WidgetID(),
          parent,
          rowID,
          key,
          false)
    , api_(api)
    , listeners_({
          {UnallocatedCString{api_.Endpoints().AccountUpdate()},
           new MessageProcessor<IssuerItem>(&IssuerItem::process_account)},
      })
    , name_{std::get<1>(key_)}
    , connection_{std::get<0>(key_)}
    , issuer_{api_.Wallet().Internal().Issuer(parent.NymID(), rowID)}
    , currency_{currency}
{
    assert_false(nullptr == issuer_);

    setup_listeners(api_, listeners_);
    startup_ = std::make_unique<std::thread>(&IssuerItem::startup, this);

    assert_false(nullptr == startup_);
}

auto IssuerItem::Debug() const noexcept -> UnallocatedCString
{
    return issuer_->toString();
}

auto IssuerItem::construct_row(
    const IssuerItemRowID& id,
    const IssuerItemSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::AccountSummaryItem(*this, api_, id, index, custom);
}

auto IssuerItem::Name() const noexcept -> UnallocatedCString
{
    const auto lock = sLock{shared_lock_};

    return name_;
}

void IssuerItem::process_account(const identifier::Account& accountID) noexcept
{
    const auto account = api_.Wallet().Internal().Account(accountID);

    if (false == bool(account)) { return; }

    auto name = String::Factory();
    account.get().GetName(name);
    const IssuerItemRowID rowID{accountID, currency_};
    const IssuerItemSortKey sortKey{name->Get()};
    CustomData custom{};
    custom.emplace_back(new Amount(account.get().GetBalance()));
    add_item(rowID, sortKey, custom);
}

void IssuerItem::process_account(const Message& message) noexcept
{
    wait_for_startup();
    const auto body = message.Payload();

    assert_true(2 < message.Payload().size());

    const auto accountID = api_.Factory().AccountIDFromZMQ(body[1].Bytes());

    assert_false(accountID.empty());

    const auto rowID = IssuerItemRowID{
        accountID, {api_.Storage().Internal().AccountUnit(accountID)}};
    const auto issuerID = api_.Storage().Internal().AccountIssuer(accountID);

    if (issuerID == issuer_->IssuerID()) { process_account(accountID); }
}

void IssuerItem::refresh_accounts() noexcept
{
    const auto blank = identifier::UnitDefinition{};
    const auto accounts = issuer_->AccountList(currency_, blank);
    LogDetail()()("Loading ")(accounts.size())(" accounts.").Flush();

    for (const auto& id : accounts) { process_account(id); }

    UnallocatedSet<IssuerItemRowID> active{};
    std::ranges::transform(
        accounts,
        std::inserter(active, active.end()),
        [&](const auto& in) -> IssuerItemRowID {
            return {in, currency_};
        });
    delete_inactive(active);
}

auto IssuerItem::reindex(const AccountSummarySortKey& key, CustomData&) noexcept
    -> bool
{
    auto lock = eLock{shared_lock_};
    key_ = key;
    connection_.store(std::get<0>(key_));
    lock.unlock();
    refresh_accounts();

    return true;
}

void IssuerItem::startup() noexcept
{
    refresh_accounts();
    finish_startup();
}

IssuerItem::~IssuerItem()
{
    for (const auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
