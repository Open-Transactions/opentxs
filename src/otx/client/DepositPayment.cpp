// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/client/DepositPayment.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
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
        LogError()()("Invalid payment").Flush();
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
                LogError()()("Failed to schedule deposit payment").Flush();
                error = true;
                repeat = true;

                goto exit;
            }

            auto value = future.get();
            const auto [result, pMessage] = value;

            if (otx::LastReplyStatus::MessageSuccess == result) {
                LogVerbose()()("Deposit success").Flush();
                result_ = std::move(value);
                error = false;
                repeat = false;

                goto exit;
            } else {
                LogError()()("Deposit failed").Flush();
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
            LogError()()("Invalid state").Flush();
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
    -> identifier::Account
{
    Lock lock(payment_tasks_.GetAccountLock(unit));
    const auto accounts =
        parent_.api().Storage().Internal().AccountsByContract(unit);

    if (1 < accounts.size()) {
        LogError()()("Too many accounts to automatically deposit payment")
            .Flush();

        return {};
    }

    if (1 == accounts.size()) { return *accounts.begin(); }

    try {
        parent_.api().Wallet().Internal().UnitDefinition(unit);
    } catch (...) {
        LogTrace()()("Downloading unit definition").Flush();
        parent_.DownloadUnitDefinition(unit);

        return {};
    }

    LogVerbose()()("Registering account for deposit").Flush();

    auto [taskid, future] = parent_.RegisterAccount({"", unit});

    if (0 == taskid) {
        LogError()()("Failed to schedule register account").Flush();

        return {};
    }

    result_ = future.get();
    const auto [result, pMessage] = result_;

    if (otx::LastReplyStatus::MessageSuccess != result) {
        LogError()()("Failed to send register account message").Flush();

        return {};
    }

    if (false == bool(pMessage)) {
        LogError()()("Invalid register account reply").Flush();

        return {};
    }

    const auto& message = *pMessage;
    const auto accountID =
        parent_.api().Factory().AccountIDFromBase58(message.acct_id_->Bytes());

    if (accountID.empty()) {
        LogError()()("Failed to get account id").Flush();
    } else {
        LogVerbose()()("Registered new account ")(
            accountID, parent_.api().Crypto())
            .Flush();
    }

    return accountID;
}

DepositPayment::~DepositPayment() { Stop(); }
}  // namespace opentxs::otx::client::implementation
