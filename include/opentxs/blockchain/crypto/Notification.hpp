// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

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
class Subaccount;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Notification : public Subaccount
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Notification&;

    auto LocalPaymentCode() const noexcept -> const opentxs::PaymentCode&;

    OPENTXS_NO_EXPORT Notification(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    Notification() = delete;
    Notification(const Notification& rhs) noexcept;
    Notification(Notification&& rhs) noexcept;
    auto operator=(const Notification&) -> Notification& = delete;
    auto operator=(Notification&&) -> Notification& = delete;

    ~Notification() override;
};
}  // namespace opentxs::blockchain::crypto
