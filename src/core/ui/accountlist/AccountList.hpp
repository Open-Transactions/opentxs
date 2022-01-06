// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "core/ui/base/List.hpp"
#include "core/ui/base/Widget.hpp"
#include "internal/core/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/ui/AccountList.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

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
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountListList = List<
    AccountListExternalInterface,
    AccountListInternalInterface,
    AccountListRowID,
    AccountListRowInterface,
    AccountListRowInternal,
    AccountListRowBlank,
    AccountListSortKey,
    AccountListPrimaryID>;

class AccountList final : public AccountListList, Worker<AccountList>
{
public:
    AccountList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

    ~AccountList() final;

private:
    friend Worker<AccountList>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        new_blockchain = value(WorkType::BlockchainAccountCreated),
        custodial = value(WorkType::AccountUpdated),
        updated_blockchain = value(WorkType::BlockchainBalance),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

#if OT_BLOCKCHAIN
    OTZMQListenCallback blockchain_balance_cb_;
    OTZMQDealerSocket blockchain_balance_;
#endif  // OT_BLOCKCHAIN

    auto construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
#if OT_BLOCKCHAIN
    auto subscribe(const blockchain::Type chain) const noexcept -> void;
#endif  // OT_BLOCKCHAIN

    auto pipeline(const Message& in) noexcept -> void;
    auto process_account(const Identifier& id) noexcept -> void;
    auto process_account(const Identifier& id, const Amount balance) noexcept
        -> void;
    auto process_account(
        const Identifier& id,
        const Amount balance,
        const UnallocatedCString& name) noexcept -> void;
    auto process_account(const Message& message) noexcept -> void;
#if OT_BLOCKCHAIN
    auto process_blockchain_account(const Message& message) noexcept -> void;
    auto process_blockchain_balance(const Message& message) noexcept -> void;
#endif  // OT_BLOCKCHAIN
    auto startup() noexcept -> void;

    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    auto operator=(const AccountList&) -> AccountList& = delete;
    auto operator=(AccountList&&) -> AccountList& = delete;
};
}  // namespace opentxs::ui::implementation
