// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

namespace ui
{
class BalanceItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BalanceItemRow =
    Row<AccountActivityRowInternal,
        AccountActivityInternalInterface,
        AccountActivityRowID>;

class BalanceItem : public BalanceItemRow
{
public:
    static auto recover_workflow(CustomData& custom) noexcept
        -> const proto::PaymentWorkflow&;

    auto Confirmations() const noexcept -> int override { return 1; }
    auto Contacts() const noexcept -> std::pmr::vector<std::string> override
    {
        return contacts_;
    }
    auto DisplayAmount() const noexcept -> std::string override;
    auto Text() const noexcept -> std::string override;
    auto Timestamp() const noexcept -> Time final;
    auto Type() const noexcept -> StorageBox override { return type_; }

    ~BalanceItem() override;

protected:
    const OTNymID nym_id_;
    const std::string workflow_;
    const StorageBox type_;
    std::string text_;
    Time time_;

    static auto extract_type(const proto::PaymentWorkflow& workflow) noexcept
        -> StorageBox;

    auto get_contact_name(const identifier::Nym& nymID) const noexcept
        -> std::string;

    auto reindex(
        const implementation::AccountActivitySortKey& key,
        implementation::CustomData& custom) noexcept -> bool override;

    BalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::session::Client& api,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const std::string& text = {}) noexcept;

private:
    const OTIdentifier account_id_;
    const std::pmr::vector<std::string> contacts_;

    static auto extract_contacts(
        const api::session::Client& api,
        const proto::PaymentWorkflow& workflow) noexcept
        -> std::pmr::vector<std::string>;

    virtual auto effective_amount() const noexcept -> opentxs::Amount = 0;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    auto operator=(const BalanceItem&) -> BalanceItem& = delete;
    auto operator=(BalanceItem&&) -> BalanceItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BalanceItem>;
