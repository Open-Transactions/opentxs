// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <utility>

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
class Subaccount;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT PaymentCode : public Deterministic
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> PaymentCode&;

    auto IncomingNotificationCount() const noexcept -> std::size_t;
    auto Local() const noexcept -> const opentxs::PaymentCode&;
    /// returns incoming count, outgoing count
    auto NotificationCount() const noexcept
        -> std::pair<std::size_t, std::size_t>;
    auto OutgoingNotificationCount() const noexcept -> std::size_t;
    auto Remote() const noexcept -> const opentxs::PaymentCode&;

    OPENTXS_NO_EXPORT PaymentCode(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    PaymentCode() = delete;
    PaymentCode(const PaymentCode& rhs) noexcept;
    PaymentCode(PaymentCode&& rhs) noexcept;
    auto operator=(const PaymentCode&) -> PaymentCode& = delete;
    auto operator=(PaymentCode&&) -> PaymentCode& = delete;

    ~PaymentCode() override;
};
}  // namespace opentxs::blockchain::crypto
