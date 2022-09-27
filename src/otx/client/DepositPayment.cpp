// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "otx/client/DepositPayment.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/client/PaymentTasks.hpp"

namespace opentxs::otx::client::implementation
{
DepositPayment::DepositPayment(
    client::internal::StateMachine& parent,
    const TaskID taskID,
    const DepositPaymentTask& payment,
    PaymentTasks& paymenttasks)
    : StateMachine([this] { return deposit(); })
    , parent_(parent)
    , task_id_(taskID)
    , payment_(payment)
    , state_(Depositability::UNKNOWN)
    , result_(parent.error_result())
    , payment_tasks_(paymenttasks)
{
}

auto DepositPayment::deposit() -> bool
{
    bool error{false};
    bool repeat{true};
    auto& [unitID, accountID, pPayment] = payment_;

    if (false == bool(pPayment)) {
        LogError()(OT_PRETTY_CLASS())("Invalid payment").Flush();
        error = true;
        repeat = false;

        goto exit;
    }

    switch (state_) {
        case Depositability::UNKNOWN: {
            if (accountID.empty()) {
                state_ = Depositability::NO_ACCOUNT;
            } else {
                state_ = Depositability::READY;
            }
        } break;
        case Depositability::NO_ACCOUNT: {
            accountID = get_account_id(unitID);

            if (accountID.empty()) {
                error = true;
                repeat = true;

                goto exit;
            }

            state_ = Depositability::READY;
            [[fallthrough]];
        }
        case Depositability::READY: {
            auto [taskid, future] = parent_.DepositPayment(payment_);

            if (0 == taskid) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to schedule deposit payment")
                    .Flush();
                error = true;
                repeat = true;

                goto exit;
            }

            auto value = future.get();
            const auto [result, pMessage] = value;

            if (otx::LastReplyStatus::MessageSuccess == result) {
                LogVerbose()(OT_PRETTY_CLASS())("Deposit success").Flush();
                result_ = std::move(value);
                error = false;
                repeat = false;

                goto exit;
            } else {
                LogError()(OT_PRETTY_CLASS())("Deposit failed").Flush();
                error = true;
                repeat = false;
            }
        } break;
        case Depositability::ACCOUNT_NOT_SPECIFIED:
        case Depositability::WRONG_ACCOUNT:
        case Depositability::INVALID_INSTRUMENT:
        case Depositability::WRONG_RECIPIENT:
        case Depositability::NOT_REGISTERED:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid state").Flush();
            error = true;
            repeat = false;

            goto exit;
        }
    }

exit:
    if (false == repeat) {
        parent_.finish_task(task_id_, !error, std::move(result_));
    }

    return repeat;
}

auto DepositPayment::get_account_id(const identifier::UnitDefinition& unit)
    -> identifier::Generic
{
    Lock lock(payment_tasks_.GetAccountLock(unit));
    const auto accounts = parent_.api().Storage().AccountsByContract(unit);

    if (1 < accounts.size()) {
        LogError()(OT_PRETTY_CLASS())(
            "Too many accounts to automatically deposit payment")
            .Flush();

        return {};
    }

    if (1 == accounts.size()) { return *accounts.begin(); }

    try {
        parent_.api().Wallet().UnitDefinition(unit);
    } catch (...) {
        LogTrace()(OT_PRETTY_CLASS())("Downloading unit definition").Flush();
        parent_.DownloadUnitDefinition(unit);

        return {};
    }

    LogVerbose()(OT_PRETTY_CLASS())("Registering account for deposit").Flush();

    auto [taskid, future] = parent_.RegisterAccount({"", unit});

    if (0 == taskid) {
        LogError()(OT_PRETTY_CLASS())("Failed to schedule register account")
            .Flush();

        return {};
    }

    result_ = future.get();
    const auto [result, pMessage] = result_;

    if (otx::LastReplyStatus::MessageSuccess != result) {
        LogError()(OT_PRETTY_CLASS())("Failed to send register account message")
            .Flush();

        return {};
    }

    if (false == bool(pMessage)) {
        LogError()(OT_PRETTY_CLASS())("Invalid register account reply").Flush();

        return {};
    }

    const auto& message = *pMessage;
    const auto accountID =
        parent_.api().Factory().IdentifierFromBase58(message.acct_id_->Bytes());

    if (accountID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Failed to get account id").Flush();
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Registered new account ")(accountID)
            .Flush();
    }

    return accountID;
}

DepositPayment::~DepositPayment() { Stop(); }
}  // namespace opentxs::otx::client::implementation
