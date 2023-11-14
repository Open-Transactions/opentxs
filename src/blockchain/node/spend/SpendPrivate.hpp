// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/Funding.hpp"

#pragma once

#include <span>
#include <string_view>

#include "internal/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class SpendPrivate : public internal::Spend
{
public:
    virtual auto Funding() const noexcept -> node::Funding;
    virtual auto ID() const noexcept -> const identifier::Generic&;
    virtual auto Memo() const noexcept -> std::string_view;
    virtual auto Notifications() const noexcept -> std::span<const PaymentCode>;
    virtual auto SpendUnconfirmedChange() const noexcept -> bool;
    virtual auto SpendUnconfirmedIncoming() const noexcept -> bool;
    virtual auto Spender() const noexcept -> const identifier::Nym&;
    virtual auto SweepFromAccount() const noexcept -> bool;
    virtual auto SweepFromKey() const noexcept -> const crypto::Key&;
    virtual auto SweepFromSubaccount() const noexcept
        -> const identifier::Account&;
    virtual auto SweepToAddress() const noexcept -> std::string_view;
    virtual auto UseEnhancedNotifications() const noexcept -> bool;

    [[nodiscard]] virtual auto Notify(const PaymentCode& recipient) noexcept
        -> bool;
    [[nodiscard]] virtual auto Notify(
        std::span<const PaymentCode> recipients) noexcept -> bool;
    [[nodiscard]] virtual auto SendToAddress(
        std::string_view address,
        const Amount& amount) noexcept -> bool;
    [[nodiscard]] virtual auto SendToPaymentCode(
        const PaymentCode& recipient,
        const Amount& amount) noexcept -> bool;
    [[nodiscard]] virtual auto SetMemo(std::string_view) noexcept -> bool;
    [[nodiscard]] virtual auto SetSpendUnconfirmedChange(bool value) noexcept
        -> bool;
    [[nodiscard]] virtual auto SetSpendUnconfirmedIncoming(bool value) noexcept
        -> bool;
    [[nodiscard]] virtual auto SetSweepFromAccount(bool value) noexcept -> bool;
    [[nodiscard]] virtual auto SetSweepFromKey(const crypto::Key& key) noexcept
        -> bool;
    [[nodiscard]] virtual auto SetSweepFromSubaccount(
        const identifier::Account& aubaccount) noexcept -> bool;
    [[nodiscard]] virtual auto SetSweepToAddress(
        std::string_view address) noexcept -> bool;

    [[nodiscard]] virtual auto SetUseEnhancedNotifications(bool value) noexcept
        -> bool;

    SpendPrivate() noexcept;
    SpendPrivate(const SpendPrivate&) = delete;
    SpendPrivate(SpendPrivate&&) = delete;
    auto operator=(const SpendPrivate&) -> SpendPrivate& = delete;
    auto operator=(SpendPrivate&&) -> SpendPrivate& = delete;

    ~SpendPrivate() override;
};
}  // namespace opentxs::blockchain::node
