// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Notification;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Notification : virtual public Subaccount
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalNotification() const noexcept
        -> const internal::Notification& = 0;
    virtual auto LocalPaymentCode() const noexcept
        -> const opentxs::PaymentCode& = 0;

    OPENTXS_NO_EXPORT virtual auto InternalNotification() noexcept
        -> internal::Notification& = 0;

    Notification(const Notification&) = delete;
    Notification(Notification&&) = delete;
    auto operator=(const Notification&) -> Notification& = delete;
    auto operator=(Notification&&) -> Notification& = delete;

    OPENTXS_NO_EXPORT ~Notification() override = default;

protected:
    Notification() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
