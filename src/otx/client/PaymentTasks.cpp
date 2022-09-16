// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "otx/client/PaymentTasks.hpp"  // IWYU pragma: associated

#include <chrono>
#include <future>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/client/DepositPayment.hpp"

namespace opentxs::otx::client::implementation
{
PaymentTasks::PaymentTasks(client::internal::StateMachine& parent)
    : StateMachine([this] { return cleanup(); })
    , parent_(parent)
    , tasks_{}
    , unit_lock_{}
    , account_lock_{}
{
}

auto PaymentTasks::cleanup() -> bool
{
    UnallocatedVector<TaskMap::iterator> finished;

    Lock lock(decision_lock_);

    for (auto i = tasks_.begin(); i != tasks_.end(); ++i) {
        auto& task = i->second;
        auto future = task.Wait();
        auto status = future.wait_for(10ns);

        if (std::future_status::ready == status) {
            LogInsane()(OT_PRETTY_CLASS())("Task for ")(i->first)(" is done")
                .Flush();

            finished.emplace_back(i);
        }
    }

    lock.unlock();

    for (auto& i : finished) {
        lock.lock();
        tasks_.erase(i);
        lock.unlock();
    }

    lock.lock();

    if (0 == tasks_.size()) { return false; }

    return true;
}

auto PaymentTasks::error_task() -> PaymentTasks::BackgroundTask
{
    BackgroundTask output{0, Future{}};

    return output;
}

auto PaymentTasks::GetAccountLock(const identifier::UnitDefinition& unit)
    -> std::mutex&
{
    Lock lock(unit_lock_);

    return account_lock_[unit];
}

auto PaymentTasks::get_payment_id(const OTPayment& payment) const
    -> identifier::Generic
{
    auto output = identifier::Generic{};

    switch (payment.GetType()) {
        case OTPayment::CHEQUE: {
            auto pCheque = parent_.api().Factory().InternalSession().Cheque();

            OT_ASSERT(pCheque);

            auto& cheque = *pCheque;
            const auto loaded =
                cheque.LoadContractFromString(payment.Payment());

            if (false == loaded) {
                LogError()(OT_PRETTY_CLASS())("Invalid cheque.").Flush();

                return output;
            }

            output = parent_.api().Factory().Internal().Identifier(cheque);

            return output;
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown payment type ")(
                OTPayment::GetTypeString(payment.GetType()))
                .Flush();

            return output;
        }
    }
}

auto PaymentTasks::PaymentTasks::Queue(const DepositPaymentTask& task)
    -> PaymentTasks::BackgroundTask
{
    const auto& pPayment = std::get<2>(task);

    if (false == bool(pPayment)) {
        LogError()(OT_PRETTY_CLASS())("Invalid payment").Flush();

        return error_task();
    }

    const auto id = get_payment_id(*pPayment);
    Lock lock(decision_lock_);

    if (0 < tasks_.count(id)) {
        LogVerbose()("Payment ")(id)(" already queued").Flush();

        return error_task();
    }

    const auto taskID = parent_.next_task_id();
    auto [it, success] = tasks_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(parent_, taskID, task, *this));

    if (false == success) {
        LogError()(OT_PRETTY_CLASS())("Failed to start queue for payment ")(id)
            .Flush();

        return error_task();
    } else {
        LogTrace()(OT_PRETTY_CLASS())("Started deposit task for ")(id).Flush();
        it->second.Trigger();
    }

    auto output = parent_.start_task(taskID, true);
    trigger(lock);

    return output;
}
}  // namespace opentxs::otx::client::implementation
