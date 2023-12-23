// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/identifier/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace internal
{
class Spend;
}  // namespace internal

class SpendPrivate;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

class Amount;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT Spend
{
public:
    auto Funding() const noexcept -> node::Funding;
    auto ID() const noexcept -> const identifier::Generic&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Spend&;
    auto Memo() const noexcept -> std::string_view;
    auto Notifications() const noexcept -> std::span<const PaymentCode>;
    auto SpendUnconfirmedChange() const noexcept -> bool;
    auto SpendUnconfirmedIncoming() const noexcept -> bool;
    auto Spender() const noexcept -> const identifier::Nym&;
    auto SweepFromAccount() const noexcept -> bool;
    auto SweepFromKey() const noexcept -> const crypto::Key&;
    auto SweepFromSubaccount() const noexcept -> const identifier::Account&;
    auto SweepToAddress() const noexcept -> std::string_view;
    auto UseEnhancedNotifications() const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Spend&;
    [[nodiscard]] auto Notify(const PaymentCode& recipient) noexcept -> bool;
    [[nodiscard]] auto Notify(std::span<const PaymentCode> recipients) noexcept
        -> bool;
    [[nodiscard]] auto SendToAddress(
        std::string_view address,
        const Amount& amount) noexcept -> bool;
    [[nodiscard]] auto SendToPaymentCode(
        const PaymentCode& recipient,
        const Amount& amount) noexcept -> bool;
    [[nodiscard]] auto SetMemo(std::string_view) noexcept -> bool;
    [[nodiscard]] auto SetSpendUnconfirmedChange(bool value) noexcept -> bool;
    [[nodiscard]] auto SetSpendUnconfirmedIncoming(bool value) noexcept -> bool;
    [[nodiscard]] auto SetSweepFromAccount(bool value) noexcept -> bool;
    [[nodiscard]] auto SetSweepFromKey(const crypto::Key& key) noexcept -> bool;
    [[nodiscard]] auto SetSweepFromSubaccount(
        const identifier::Account& aubaccount) noexcept -> bool;
    [[nodiscard]] auto SetSweepToAddress(std::string_view address) noexcept
        -> bool;
    [[nodiscard]] auto SetUseEnhancedNotifications(bool value) noexcept -> bool;

    Spend() noexcept;
    OPENTXS_NO_EXPORT Spend(SpendPrivate* imp) noexcept;
    Spend(const Spend&) = delete;
    Spend(Spend&& rhs) noexcept;
    auto operator=(const Spend&) -> Spend& = delete;
    auto operator=(Spend&&) -> Spend& = delete;

    ~Spend();

private:
    SpendPrivate* imp_;
};
}  // namespace opentxs::blockchain::node
