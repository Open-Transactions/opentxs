// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct PaymentCode;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT PaymentCode : virtual public Deterministic
{
public:
    virtual auto AddNotification(const Txid& tx) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto InternalPaymentCode() const noexcept
        -> internal::PaymentCode& = 0;
    virtual auto IsNotified() const noexcept -> bool = 0;
    virtual auto Local() const noexcept -> const opentxs::PaymentCode& = 0;
    virtual auto ReorgNotification(const Txid& tx) const noexcept -> bool = 0;
    virtual auto Remote() const noexcept -> const opentxs::PaymentCode& = 0;

    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode& = delete;
    auto operator=(PaymentCode&&) -> PaymentCode& = delete;

    OPENTXS_NO_EXPORT ~PaymentCode() override = default;

protected:
    PaymentCode() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
