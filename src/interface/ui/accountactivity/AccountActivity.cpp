// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/AccountActivity.hpp"  // IWYU pragma: associated

#include <future>
#include <utility>

#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Work.hpp"

namespace opentxs::ui::implementation
{
AccountActivity::AccountActivity(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const AccountType type,
    const SimpleCallback& cb) noexcept
    : AccountActivityList(api, nymID, cb, true)
    , Worker(api, {})
    , callbacks_()
    , balance_(0)
    , account_id_(accountID)
    , type_(type)
    , contract_([&] {
        try {

            return api.Wallet().UnitDefinition(
                api.Storage().AccountContract(account_id_));
        } catch (...) {

            return api.Factory().UnitDefinition();
        }
    }())
    , notary_([&] {
        try {

            return api.Wallet().Server(
                api.Storage().AccountServer(account_id_));
        } catch (...) {

            return api.Factory().ServerContract();
        }
    }())
    , qt_(nullptr)
{
    init_qt();
}

auto AccountActivity::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = {};
}

auto AccountActivity::construct_row(
    const AccountActivityRowID& id,
    const AccountActivitySortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BalanceItem(
        *this,
        Widget::Widget::api_,
        id,
        index,
        custom,
        primary_id_,
        account_id_);
}

auto AccountActivity::init(Endpoints endpoints) noexcept -> void
{
    init_executor(std::move(endpoints));
    pipeline_.Push(MakeWork(OT_ZMQ_INIT_SIGNAL));
}

auto AccountActivity::notify_balance(opentxs::Amount balance) const noexcept
    -> void
{
    const auto display = display_balance(balance);
    auto lock = Lock{callbacks_.lock_};
    const auto& cb = callbacks_.cb_;

    if (cb.balance_) { cb.balance_(display.c_str()); }
    if (cb.polarity_) { cb.polarity_(polarity(balance)); }

    UpdateNotify();
}

auto AccountActivity::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = cb;
}

AccountActivity::~AccountActivity()
{
    wait_for_startup();
    signal_shutdown().get();
    shutdown_qt();
}
}  // namespace opentxs::ui::implementation
