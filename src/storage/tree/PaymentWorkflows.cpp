// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PaymentWorkflows.hpp"

#include "opentxs/core/Log.hpp"

#include "storage/Plugin.hpp"

#define CURRENT_VERSION 2
#define TYPE_VERSION 2
#define INDEX_VERSION 1

//#define OT_METHOD "opentxs::storage::PaymentWorkflows::"

namespace opentxs::storage
{
PaymentWorkflows::PaymentWorkflows(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
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
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

void PaymentWorkflows::add_state_index(
    const Lock& lock,
    const std::string& workflowID,
    proto::PaymentWorkflowType type,
    proto::PaymentWorkflowState state)
{
    OT_ASSERT(verify_write_lock(lock))
    OT_ASSERT(false == workflowID.empty())
    OT_ASSERT(proto::PAYMENTWORKFLOWTYPE_ERROR != type)
    OT_ASSERT(proto::PAYMENTWORKFLOWSTATE_ERROR != state)

    const State key{type, state};
    workflow_state_map_.emplace(workflowID, key);
    type_workflow_map_[type].emplace(workflowID);
    state_workflow_map_[key].emplace(workflowID);
}

bool PaymentWorkflows::Delete(const std::string& id)
{
    Lock lock(write_lock_);
    delete_by_value(id);
    lock.unlock();

    return delete_item(id);
}

void PaymentWorkflows::delete_by_value(const std::string& value)
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

PaymentWorkflows::State PaymentWorkflows::GetState(
    const std::string& workflowID) const
{
    State output{proto::PAYMENTWORKFLOWTYPE_ERROR,
                 proto::PAYMENTWORKFLOWSTATE_ERROR};
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

void PaymentWorkflows::init(const std::string& hash)
{
    std::shared_ptr<proto::StoragePaymentWorkflows> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load workflow index file.")
            .Flush();
        OT_FAIL
    }

    version_ = serialized->version();

    if (CURRENT_VERSION > version_) { version_ = CURRENT_VERSION; }

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
        add_state_index(lock, workflowID, type, state);
    }
}

PaymentWorkflows::Workflows PaymentWorkflows::ListByAccount(
    const std::string& accountID) const
{
    Lock lock(write_lock_);
    const auto it = account_workflow_map_.find(accountID);

    if (account_workflow_map_.end() == it) { return {}; }

    return it->second;
}

PaymentWorkflows::Workflows PaymentWorkflows::ListByUnit(
    const std::string& accountID) const
{
    Lock lock(write_lock_);
    const auto it = unit_workflow_map_.find(accountID);

    if (unit_workflow_map_.end() == it) { return {}; }

    return it->second;
}

PaymentWorkflows::Workflows PaymentWorkflows::ListByState(
    proto::PaymentWorkflowType type,
    proto::PaymentWorkflowState state) const
{
    Lock lock(write_lock_);
    const auto it = state_workflow_map_.find(State{type, state});

    if (state_workflow_map_.end() == it) { return {}; }

    return it->second;
}

bool PaymentWorkflows::Load(
    const std::string& id,
    std::shared_ptr<proto::PaymentWorkflow>& output,
    const bool checking) const
{
    std::string alias;

    return load_proto<proto::PaymentWorkflow>(id, output, alias, checking);
}

std::string PaymentWorkflows::LookupBySource(const std::string& sourceID) const
{
    Lock lock(write_lock_);
    const auto it = item_workflow_map_.find(sourceID);

    if (item_workflow_map_.end() == it) { return {}; }

    return it->second;
}

void PaymentWorkflows::reindex(
    const Lock& lock,
    const std::string& workflowID,
    const proto::PaymentWorkflowType type,
    const proto::PaymentWorkflowState newState,
    proto::PaymentWorkflowState& state)
{
    OT_ASSERT(verify_write_lock(lock))
    OT_ASSERT(false == workflowID.empty())

    const State oldKey{type, state};
    auto& oldSet = state_workflow_map_[oldKey];
    oldSet.erase(workflowID);

    if (0 == oldSet.size()) { state_workflow_map_.erase(oldKey); }

    state = newState;

    OT_ASSERT(proto::PAYMENTWORKFLOWTYPE_ERROR != type)
    OT_ASSERT(proto::PAYMENTWORKFLOWSTATE_ERROR != state)

    const State newKey{type, newState};
    state_workflow_map_[newKey].emplace(workflowID);
}

bool PaymentWorkflows::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StoragePaymentWorkflows PaymentWorkflows::serialize() const
{
    proto::StoragePaymentWorkflows serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                item.first, item.second, *serialized.add_workflow());
        }
    }

    for (const auto& [item, workflow] : item_workflow_map_) {
        auto& newIndex = *serialized.add_items();
        newIndex.set_version(INDEX_VERSION);
        newIndex.set_workflow(workflow);
        newIndex.set_item(item);
    }

    for (const auto& [account, workflowSet] : account_workflow_map_) {
        OT_ASSERT(false == account.empty())

        for (const auto& workflow : workflowSet) {
            OT_ASSERT(false == workflow.empty())

            auto& newAccount = *serialized.add_accounts();
            newAccount.set_version(INDEX_VERSION);
            newAccount.set_workflow(workflow);
            newAccount.set_item(account);
        }
    }

    for (const auto& [unit, workflowSet] : unit_workflow_map_) {
        OT_ASSERT(false == unit.empty())

        for (const auto& workflow : workflowSet) {
            OT_ASSERT(false == workflow.empty())

            auto& newUnit = *serialized.add_units();
            newUnit.set_version(INDEX_VERSION);
            newUnit.set_workflow(workflow);
            newUnit.set_item(unit);
        }
    }

    for (const auto& [workflow, stateTuple] : workflow_state_map_) {
        OT_ASSERT(false == workflow.empty())

        const auto& [type, state] = stateTuple;

        OT_ASSERT(proto::PAYMENTWORKFLOWTYPE_ERROR != type)
        OT_ASSERT(proto::PAYMENTWORKFLOWSTATE_ERROR != state)

        auto& newIndex = *serialized.add_types();
        newIndex.set_version(TYPE_VERSION);
        newIndex.set_workflow(workflow);
        newIndex.set_type(type);
        newIndex.set_state(state);
    }

    for (const auto& archived : archived_) {
        OT_ASSERT(false == archived.empty())

        serialized.add_archived(archived);
    }

    return serialized;
}

bool PaymentWorkflows::Store(
    const proto::PaymentWorkflow& data,
    std::string& plaintext)
{
    Lock lock(write_lock_);
    std::string alias;
    const auto& id = data.id();
    delete_by_value(id);

    for (const auto& source : data.source()) {
        item_workflow_map_.emplace(source.id(), id);
    }

    const auto it = workflow_state_map_.find(id);

    if (workflow_state_map_.end() == it) {
        add_state_index(lock, id, data.type(), data.state());
    } else {
        auto& [type, state] = it->second;
        reindex(lock, id, type, data.state(), state);
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
