// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <utility>

#include "internal/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
class PaymentCode : virtual public Deterministic
{
public:
    static auto Blank() noexcept -> PaymentCode&;
    static auto GetID(
        const api::Session& api,
        const crypto::Target target,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote) noexcept -> identifier::Account;

    virtual auto AddIncomingNotification(
        const block::TransactionHash& tx) const noexcept -> bool;
    virtual auto AddNotification(
        const block::TransactionHash& tx) const noexcept -> bool;
    virtual auto IncomingNotificationCount() const noexcept -> std::size_t;
    virtual auto Local() const noexcept -> const opentxs::PaymentCode&;
    virtual auto NotificationCount() const noexcept
        -> std::pair<std::size_t, std::size_t>;
    virtual auto OutgoingNotificationCount() const noexcept -> std::size_t;
    virtual auto Remote() const noexcept -> const opentxs::PaymentCode&;
    virtual auto ReorgNotification(
        const block::TransactionHash& tx) const noexcept -> bool;

    PaymentCode() = default;
    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode& = delete;
    auto operator=(PaymentCode&&) -> PaymentCode& = delete;

    ~PaymentCode() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
