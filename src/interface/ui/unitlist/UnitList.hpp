// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using UnitListList = List<
    UnitListExternalInterface,
    UnitListInternalInterface,
    UnitListRowID,
    UnitListRowInterface,
    UnitListRowInternal,
    UnitListRowBlank,
    UnitListSortKey,
    UnitListPrimaryID>;

class UnitList final : public UnitListList
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }

    UnitList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    UnitList() = delete;
    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    auto operator=(const UnitList&) -> UnitList& = delete;
    auto operator=(UnitList&&) -> UnitList& = delete;

    ~UnitList() final;

private:
    OTZMQListenCallback blockchain_balance_cb_;
    OTZMQDealerSocket blockchain_balance_;
    const ListenerDefinitions listeners_;

    auto construct_row(
        const UnitListRowID& id,
        const UnitListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto process_account(const Message& message) noexcept -> void;
    auto process_account(const identifier::Account& id) noexcept -> void;
    auto process_blockchain_balance(const Message& message) noexcept -> void;
    auto process_unit(const UnitListRowID& id) noexcept -> void;
    auto setup_listeners(
        const api::session::Client& api,
        const ListenerDefinitions& definitions) noexcept -> void final;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
