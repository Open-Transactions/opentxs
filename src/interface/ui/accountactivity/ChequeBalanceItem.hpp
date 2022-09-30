// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class ChequeBalanceItem final : public BalanceItem
{
public:
    auto Amount() const noexcept -> opentxs::Amount final
    {
        return effective_amount();
    }
    auto Memo() const noexcept -> UnallocatedCString final;
    auto UUID() const noexcept -> UnallocatedCString final;
    auto Workflow() const noexcept -> UnallocatedCString final
    {
        return workflow_;
    }

    ChequeBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::session::Client& api,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        CustomData& custom,
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) noexcept;
    ChequeBalanceItem() = delete;
    ChequeBalanceItem(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem(ChequeBalanceItem&&) = delete;
    auto operator=(const ChequeBalanceItem&) -> ChequeBalanceItem& = delete;
    auto operator=(ChequeBalanceItem&&) -> ChequeBalanceItem& = delete;

    ~ChequeBalanceItem() final = default;

private:
    std::unique_ptr<const opentxs::Cheque> cheque_;

    auto effective_amount() const noexcept -> opentxs::Amount final;

    auto reindex(
        const implementation::AccountActivitySortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
    auto startup(
        const proto::PaymentWorkflow workflow,
        const proto::PaymentEvent event) noexcept -> bool;
};
}  // namespace opentxs::ui::implementation
