// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "core/StateMachine.hpp"
#include "internal/otx/client/Client.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "otx/client/DepositPayment.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace otx
{
namespace client
{
namespace implementation
{
class DepositPayment;
}  // namespace implementation
}  // namespace client
}  // namespace otx
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client::implementation
{
class PaymentTasks final : public opentxs::internal::StateMachine
{
public:
    using BackgroundTask = api::session::OTX::BackgroundTask;

    auto GetAccountLock(const identifier::UnitDefinition& unit) -> std::mutex&;
    auto Queue(const DepositPaymentTask& task) -> BackgroundTask;

    PaymentTasks(client::internal::StateMachine& parent);
    ~PaymentTasks() final = default;

private:
    using Future = api::session::OTX::Future;
    using TaskMap =
        UnallocatedMap<OTIdentifier, implementation::DepositPayment>;

    static auto error_task() -> BackgroundTask;

    client::internal::StateMachine& parent_;
    TaskMap tasks_;
    std::mutex unit_lock_;
    UnallocatedMap<OTUnitID, std::mutex> account_lock_;

    auto cleanup() -> bool;
    auto get_payment_id(const OTPayment& payment) const -> OTIdentifier;

    PaymentTasks() = delete;
};
}  // namespace opentxs::otx::client::implementation
