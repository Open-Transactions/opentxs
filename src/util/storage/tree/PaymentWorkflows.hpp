// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StoragePaymentWorkflows.pb.h>
#include <functional>
#include <memory>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace protobuf
{
class PaymentWorkflow;
}  // namespace protobuf

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Nym;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class PaymentWorkflows final : public Node
{
public:
    using State = std::pair<
        otx::client::PaymentWorkflowType,
        otx::client::PaymentWorkflowState>;
    using Workflows = UnallocatedSet<identifier::Generic>;

    auto GetState(const identifier::Generic& workflowID) const -> State;
    auto ListByAccount(const identifier::Account& accountID) const -> Workflows;
    auto ListByState(
        otx::client::PaymentWorkflowType type,
        otx::client::PaymentWorkflowState state) const -> Workflows;
    auto ListByUnit(const identifier::UnitDefinition& unitID) const
        -> Workflows;
    auto Load(
        const identifier::Generic& id,
        std::shared_ptr<protobuf::PaymentWorkflow>& output,
        ErrorReporting checking) const -> bool;
    auto LookupBySource(const identifier::Generic& sourceID) const
        -> identifier::Generic;

    auto Delete(const identifier::Generic& id) -> bool;
    auto Store(
        const protobuf::PaymentWorkflow& data,
        UnallocatedCString& plaintext) -> bool;

    PaymentWorkflows() = delete;
    PaymentWorkflows(const PaymentWorkflows&) = delete;
    PaymentWorkflows(PaymentWorkflows&&) = delete;
    auto operator=(const PaymentWorkflows&) -> PaymentWorkflows = delete;
    auto operator=(PaymentWorkflows&&) -> PaymentWorkflows = delete;

    ~PaymentWorkflows() final = default;

private:
    friend Nym;

    static constexpr auto current_version_ = VersionNumber{3};
    static constexpr auto type_version_ = VersionNumber{3};
    static constexpr auto index_version_ = VersionNumber{1};

    Workflows archived_;
    UnallocatedMap<identifier::Generic, identifier::Generic> item_workflow_map_;
    UnallocatedMap<identifier::Account, Workflows> account_workflow_map_;
    UnallocatedMap<identifier::UnitDefinition, Workflows> unit_workflow_map_;
    UnallocatedMap<identifier::Generic, State> workflow_state_map_;
    UnallocatedMap<otx::client::PaymentWorkflowType, Workflows>
        type_workflow_map_;
    UnallocatedMap<State, Workflows> state_workflow_map_;

    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> protobuf::StoragePaymentWorkflows;

    void add_state_index(
        const Lock& lock,
        const identifier::Generic& workflowID,
        otx::client::PaymentWorkflowType type,
        otx::client::PaymentWorkflowState state);
    void delete_by_value(const identifier::Generic& value);
    auto init(const Hash& hash) noexcept(false) -> void final;
    void reindex(
        const Lock& lock,
        const identifier::Generic& workflowID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState newState,
        otx::client::PaymentWorkflowState& state);
    auto upgrade(const Lock& lock) noexcept -> bool final;

    PaymentWorkflows(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& key);
};
}  // namespace opentxs::storage::tree
