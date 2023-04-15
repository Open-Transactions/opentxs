// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace identifier
{
class Generic;
class Nym;
class UnitDefinition;
}  // namespace identifier
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    auto API() const noexcept -> const api::Session& final { return api_; }

    AccountList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    auto operator=(const AccountList&) -> AccountList& = delete;
    auto operator=(AccountList&&) -> AccountList& = delete;

    ~AccountList() final;

private:
    friend Worker<AccountList>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        custodial = value(WorkType::AccountUpdated),
        blockchain = value(WorkType::BlockchainAccountCreated),
        balance = value(WorkType::BlockchainBalance),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    static auto print(Work type) noexcept -> const char*;

    UnallocatedSet<blockchain::Type> chains_;

    auto construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto subscribe(const blockchain::Type chain) const noexcept -> void;

    auto load_blockchain() noexcept -> void;
    auto load_blockchain_account(identifier::Generic&& id) noexcept -> void;
    auto load_blockchain_account(blockchain::Type chain) noexcept -> void;
    auto load_blockchain_account(
        identifier::Generic&& id,
        blockchain::Type chain) noexcept -> void;
    auto load_blockchain_account(
        identifier::Generic&& id,
        blockchain::Type chain,
        Amount&& balance) noexcept -> void;
    auto load_custodial() noexcept -> void;
    auto load_custodial_account(identifier::Generic&& id) noexcept -> void;
    auto load_custodial_account(
        identifier::Generic&& id,
        Amount&& balance) noexcept -> void;
    auto load_custodial_account(
        identifier::Generic&& id,
        identifier::UnitDefinition&& contract,
        UnitType type,
        Amount&& balance,
        UnallocatedCString&& name) noexcept -> void;
    auto pipeline(Message&& in) noexcept -> void;
    auto process_custodial(Message&& message) noexcept -> void;
    auto process_blockchain(Message&& message) noexcept -> void;
    auto process_blockchain_balance(Message&& message) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
