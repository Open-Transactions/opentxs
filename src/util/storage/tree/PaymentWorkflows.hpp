// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "serialization/protobuf/StoragePaymentWorkflows.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class PaymentWorkflow;
}  // namespace proto

namespace storage
{
class Driver;
class Nym;
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTENd(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class PaymentWorkflows final : public Node
{
public:
    using State = std::pair<
        otx::client::PaymentWorkflowType,
        otx::client::PaymentWorkflowState>;
    using Workflows = UnallocatedSet<UnallocatedCString>;

    auto GetState(const UnallocatedCString& workflowID) const -> State;
    auto ListByAccount(const UnallocatedCString& accountID) const -> Workflows;
    auto ListByState(
        otx::client::PaymentWorkflowType type,
        otx::client::PaymentWorkflowState state) const -> Workflows;
    auto ListByUnit(const UnallocatedCString& unitID) const -> Workflows;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::PaymentWorkflow>& output,
        const bool checking) const -> bool;
    auto LookupBySource(const UnallocatedCString& sourceID) const
        -> UnallocatedCString;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto Store(
        const proto::PaymentWorkflow& data,
        UnallocatedCString& plaintext) -> bool;

    ~PaymentWorkflows() final = default;

private:
    friend Nym;

    static constexpr auto current_version_ = VersionNumber{3};
    static constexpr auto type_version_ = VersionNumber{3};
    static constexpr auto index_version_ = VersionNumber{1};
    static constexpr auto hash_version_ = VersionNumber{2};

    Workflows archived_;
    UnallocatedMap<UnallocatedCString, UnallocatedCString> item_workflow_map_;
    UnallocatedMap<UnallocatedCString, Workflows> account_workflow_map_;
    UnallocatedMap<UnallocatedCString, Workflows> unit_workflow_map_;
    UnallocatedMap<UnallocatedCString, State> workflow_state_map_;
    UnallocatedMap<otx::client::PaymentWorkflowType, Workflows>
        type_workflow_map_;
    UnallocatedMap<State, Workflows> state_workflow_map_;

    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StoragePaymentWorkflows;

    void add_state_index(
        const Lock& lock,
        const UnallocatedCString& workflowID,
        otx::client::PaymentWorkflowType type,
        otx::client::PaymentWorkflowState state);
    void delete_by_value(const UnallocatedCString& value);
    void init(const UnallocatedCString& hash) final;
    void reindex(
        const Lock& lock,
        const UnallocatedCString& workflowID,
        const otx::client::PaymentWorkflowType type,
        const otx::client::PaymentWorkflowState newState,
        otx::client::PaymentWorkflowState& state);

    PaymentWorkflows(const Driver& storage, const UnallocatedCString& key);
    PaymentWorkflows() = delete;
    PaymentWorkflows(const PaymentWorkflows&) = delete;
    PaymentWorkflows(PaymentWorkflows&&) = delete;
    auto operator=(const PaymentWorkflows&) -> PaymentWorkflows = delete;
    auto operator=(PaymentWorkflows&&) -> PaymentWorkflows = delete;
};
}  // namespace opentxs::storage
