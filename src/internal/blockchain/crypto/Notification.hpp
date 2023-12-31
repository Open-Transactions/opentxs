// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class HDPath;
}  // namespace protobuf

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
class Notification : virtual public Subaccount
{
public:
    static auto Blank() noexcept -> Notification&;
    static auto CalculateID(
        const api::Session& api,
        const crypto::Target& target,
        const opentxs::PaymentCode& code) noexcept -> identifier::Account;

    virtual auto LocalPaymentCode() const noexcept
        -> const opentxs::PaymentCode&;
    virtual auto Path() const noexcept -> protobuf::HDPath;

    Notification() = default;
    Notification(const Notification&) = delete;
    Notification(Notification&&) = delete;
    auto operator=(const Notification&) -> Notification& = delete;
    auto operator=(Notification&&) -> Notification& = delete;

    ~Notification() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
