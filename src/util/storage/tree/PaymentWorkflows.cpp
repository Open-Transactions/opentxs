// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "util/storage/tree/PaymentWorkflows.hpp"  // IWYU pragma: associated

#include <tuple>
#include <type_traits>

#include "Proto.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/PaymentWorkflow.hpp"
#include "internal/serialization/protobuf/verify/StoragePaymentWorkflows.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "serialization/protobuf/InstrumentRevision.pb.h"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "serialization/protobuf/StoragePaymentWorkflows.pb.h"
#include "serialization/protobuf/StorageWorkflowIndex.pb.h"
#include "serialization/protobuf/StorageWorkflowType.pb.h"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage
{
PaymentWorkflows::PaymentWorkflows(
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(storage, hash)
    , archived_()
    , item_workflow_map_()
    , account_workflow_map_()
    , unit_workflow_map_()
    , workflow_state_map_()
    , type_workflow_map_()
    , state_workflow_map_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

void PaymentWorkflows::add_state_index(
    const Lock& lock,
    const UnallocatedCString& workflowID,
    otx::client::PaymentWorkflowType type,
    otx::client::PaymentWorkflowState state)
{
    OT_ASSERT(verify_write_lock(lock))
    OT_ASSERT(false == workflowID.empty())
    OT_ASSERT(otx::client::PaymentWorkflowType::Error != type)
    OT_ASSERT(otx::client::PaymentWorkflowState::Error != state)

    const State key{type, state};
    workflow_state_map_.emplace(workflowID, key);
    type_workflow_map_[type].emplace(workflowID);
    state_workflow_map_[key].emplace(workflowID);
}

auto PaymentWorkflows::Delete(const UnallocatedCString& id) -> bool
{
    Lock lock(write_lock_);
    delete_by_value(id);
    lock.unlock();

    return delete_item(id);
}

void PaymentWorkflows::delete_by_value(const UnallocatedCString& value)
{
    auto it = item_workflow_map_.begin();

    while (it != item_workflow_map_.end()) {
        if (it->second == value) {
            it = item_workflow_map_.erase(it);
        } else {
            ++it;
        }
    }
}

auto PaymentWorkflows::GetState(const UnallocatedCString& workflowID) const
    -> PaymentWorkflows::State
{
    State output{
        otx::client::PaymentWorkflowType::Error,
        otx::client::PaymentWorkflowState::Error};
    auto& [outType, outState] = output;
    Lock lock(write_lock_);
    const auto& it = workflow_state_map_.find(workflowID);
    const bool found = workflow_state_map_.end() != it;
    lock.unlock();

    if (found) {
        const auto& [type, state] = it->second;
        outType = type;
        outState = state;
    }

    return output;
}

void PaymentWorkflows::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StoragePaymentWorkflows> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogError()(OT_PRETTY_CLASS())("Failed to load workflow index file.")
            .Flush();
        OT_FAIL
    }

    init_version(current_version_, *serialized);

    for (const auto& it : serialized->workflow()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    for (const auto& it : serialized->items()) {
        item_workflow_map_.emplace(it.item(), it.workflow());
    }

    for (const auto& it : serialized->accounts()) {
        account_workflow_map_[it.item()].emplace(it.workflow());
    }

    for (const auto& it : serialized->units()) {
        unit_workflow_map_[it.item()].emplace(it.workflow());
    }

    for (const auto& it : serialized->archived()) { archived_.emplace(it); }

    Lock lock(write_lock_);

    for (const auto& it : serialized->types()) {
        const auto& workflowID = it.workflow();
        const auto& type = it.type();
        const auto& state = it.state();
        add_state_index(lock, workflowID, translate(type), translate(state));
    }
}

auto PaymentWorkflows::ListByAccount(const UnallocatedCString& accountID) const
    -> PaymentWorkflows::Workflows
{
    Lock lock(write_lock_);
    const auto it = account_workflow_map_.find(accountID);

    if (account_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::ListByUnit(const UnallocatedCString& accountID) const
    -> PaymentWorkflows::Workflows
{
    Lock lock(write_lock_);
    const auto it = unit_workflow_map_.find(accountID);

    if (unit_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::ListByState(
    otx::client::PaymentWorkflowType type,
    otx::client::PaymentWorkflowState state) const
    -> PaymentWorkflows::Workflows
{
    Lock lock(write_lock_);
    const auto it = state_workflow_map_.find(State{type, state});

    if (state_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::Load(
    const UnallocatedCString& id,
    std::shared_ptr<proto::PaymentWorkflow>& output,
    const bool checking) const -> bool
{
    UnallocatedCString alias;

    return load_proto<proto::PaymentWorkflow>(id, output, alias, checking);
}

auto PaymentWorkflows::LookupBySource(const UnallocatedCString& sourceID) const
    -> UnallocatedCString
{
    Lock lock(write_lock_);
    const auto it = item_workflow_map_.find(sourceID);

    if (item_workflow_map_.end() == it) { return {}; }

    return it->second;
}

void PaymentWorkflows::reindex(
    const Lock& lock,
    const UnallocatedCString& workflowID,
    const otx::client::PaymentWorkflowType type,
    const otx::client::PaymentWorkflowState newState,
    otx::client::PaymentWorkflowState& state)
{
    OT_ASSERT(verify_write_lock(lock))
    OT_ASSERT(false == workflowID.empty())

    const State oldKey{type, state};
    auto& oldSet = state_workflow_map_[oldKey];
    oldSet.erase(workflowID);

    if (0 == oldSet.size()) { state_workflow_map_.erase(oldKey); }

    state = newState;

    OT_ASSERT(otx::client::PaymentWorkflowType::Error != type)
    OT_ASSERT(otx::client::PaymentWorkflowState::Error != state)

    const State newKey{type, newState};
    state_workflow_map_[newKey].emplace(workflowID);
}

auto PaymentWorkflows::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto PaymentWorkflows::serialize() const -> proto::StoragePaymentWorkflows
{
    proto::StoragePaymentWorkflows serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                hash_version_,
                item.first,
                item.second,
                *serialized.add_workflow());
        }
    }

    for (const auto& [item, workflow] : item_workflow_map_) {
        auto& newIndex = *serialized.add_items();
        newIndex.set_version(index_version_);
        newIndex.set_workflow(workflow);
        newIndex.set_item(item);
    }

    for (const auto& [account, workflowSet] : account_workflow_map_) {
        OT_ASSERT(false == account.empty())

        for (const auto& workflow : workflowSet) {
            OT_ASSERT(false == workflow.empty())

            auto& newAccount = *serialized.add_accounts();
            newAccount.set_version(index_version_);
            newAccount.set_workflow(workflow);
            newAccount.set_item(account);
        }
    }

    for (const auto& [unit, workflowSet] : unit_workflow_map_) {
        OT_ASSERT(false == unit.empty())

        for (const auto& workflow : workflowSet) {
            OT_ASSERT(false == workflow.empty())

            auto& newUnit = *serialized.add_units();
            newUnit.set_version(index_version_);
            newUnit.set_workflow(workflow);
            newUnit.set_item(unit);
        }
    }

    for (const auto& [workflow, stateTuple] : workflow_state_map_) {
        OT_ASSERT(false == workflow.empty())

        const auto& [type, state] = stateTuple;

        OT_ASSERT(otx::client::PaymentWorkflowType::Error != type)
        OT_ASSERT(otx::client::PaymentWorkflowState::Error != state)

        auto& newIndex = *serialized.add_types();
        newIndex.set_version(type_version_);
        newIndex.set_workflow(workflow);
        newIndex.set_type(translate(type));
        newIndex.set_state(translate(state));
    }

    for (const auto& archived : archived_) {
        OT_ASSERT(false == archived.empty())

        serialized.add_archived(archived);
    }

    return serialized;
}

auto PaymentWorkflows::Store(
    const proto::PaymentWorkflow& data,
    UnallocatedCString& plaintext) -> bool
{
    Lock lock(write_lock_);
    UnallocatedCString alias;
    const auto& id = data.id();
    delete_by_value(id);

    for (const auto& source : data.source()) {
        item_workflow_map_.emplace(source.id(), id);
    }

    const auto it = workflow_state_map_.find(id);

    if (workflow_state_map_.end() == it) {
        add_state_index(
            lock, id, translate(data.type()), translate(data.state()));
    } else {
        auto& [type, state] = it->second;
        reindex(lock, id, type, translate(data.state()), state);
    }

    for (const auto& account : data.account()) {
        account_workflow_map_[account].emplace(id);
    }

    for (const auto& unit : data.unit()) {
        unit_workflow_map_[unit].emplace(id);
    }

    return store_proto(lock, data, id, alias, plaintext);
}
}  // namespace opentxs::storage
