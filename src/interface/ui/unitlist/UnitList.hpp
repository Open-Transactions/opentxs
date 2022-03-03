// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "1_Internal.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/interface/ui/UnitList.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
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

class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
// }  // namespace v1
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
    UnitList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

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
    auto process_account(const Identifier& id) noexcept -> void;
    auto process_blockchain_balance(const Message& message) noexcept -> void;
    auto process_unit(const UnitListRowID& id) noexcept -> void;
    auto setup_listeners(const ListenerDefinitions& definitions) noexcept
        -> void final;
    auto startup() noexcept -> void;

    UnitList() = delete;
    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    auto operator=(const UnitList&) -> UnitList& = delete;
    auto operator=(UnitList&&) -> UnitList& = delete;
};
}  // namespace opentxs::ui::implementation
