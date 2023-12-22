// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/PaymentWorkflows.hpp"  // IWYU pragma: associated

#include <InstrumentRevision.pb.h>
#include <PaymentWorkflow.pb.h>
#include <StoragePaymentWorkflows.pb.h>
#include <StorageWorkflowIndex.pb.h>
#include <StorageWorkflowType.pb.h>
#include <atomic>
#include <source_location>
#include <stdexcept>
#include <tuple>

#include "internal/api/session/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/PaymentWorkflow.hpp"
#include "internal/serialization/protobuf/verify/StoragePaymentWorkflows.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"              // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

PaymentWorkflows::PaymentWorkflows(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          current_version_)
    , archived_()
    , item_workflow_map_()
    , account_workflow_map_()
    , unit_workflow_map_()
    , workflow_state_map_()
    , type_workflow_map_()
    , state_workflow_map_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

void PaymentWorkflows::add_state_index(
    const Lock& lock,
    const identifier::Generic& workflowID,
    otx::client::PaymentWorkflowType type,
    otx::client::PaymentWorkflowState state)
{
    assert_true(verify_write_lock(lock));
    assert_false(workflowID.empty());
    assert_true(otx::client::PaymentWorkflowType::Error != type);
    assert_true(otx::client::PaymentWorkflowState::Error != state);

    const State key{type, state};
    workflow_state_map_.emplace(workflowID, key);
    type_workflow_map_[type].emplace(workflowID);
    state_workflow_map_[key].emplace(workflowID);
}

auto PaymentWorkflows::Delete(const identifier::Generic& id) -> bool
{
    auto lock = Lock{write_lock_};
    delete_by_value(id);
    lock.unlock();

    return delete_item(id);
}

void PaymentWorkflows::delete_by_value(const identifier::Generic& value)
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

auto PaymentWorkflows::GetState(const identifier::Generic& workflowID) const
    -> PaymentWorkflows::State
{
    State output{
        otx::client::PaymentWorkflowType::Error,
        otx::client::PaymentWorkflowState::Error};
    auto& [outType, outState] = output;
    auto lock = Lock{write_lock_};
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

auto PaymentWorkflows::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StoragePaymentWorkflows>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 3u:
            case 2u:
            case 1u:
            default: {
                init_map(proto.workflow());

                for (const auto& it : proto.items()) {
                    item_workflow_map_.emplace(
                        factory_.IdentifierFromBase58(it.item()),
                        factory_.IdentifierFromBase58(it.workflow()));
                }

                for (const auto& it : proto.accounts()) {
                    account_workflow_map_[factory_.AccountIDFromBase58(
                                              it.item())]
                        .emplace(factory_.IdentifierFromBase58(it.workflow()));
                }

                for (const auto& it : proto.units()) {
                    unit_workflow_map_[factory_.UnitIDFromBase58(it.item())]
                        .emplace(factory_.IdentifierFromBase58(it.workflow()));
                }

                for (const auto& it : proto.archived()) {
                    archived_.emplace(factory_.IdentifierFromBase58(it));
                }

                const auto lock = Lock{write_lock_};

                for (const auto& it : proto.types()) {
                    const auto workflowID =
                        factory_.IdentifierFromBase58(it.workflow());
                    const auto& type = it.type();
                    const auto& state = it.state();
                    add_state_index(
                        lock, workflowID, translate(type), translate(state));
                }
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto PaymentWorkflows::ListByAccount(const identifier::Account& accountID) const
    -> PaymentWorkflows::Workflows
{
    const auto lock = Lock{write_lock_};
    const auto it = account_workflow_map_.find(accountID);

    if (account_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::ListByUnit(const identifier::UnitDefinition& accountID)
    const -> PaymentWorkflows::Workflows
{
    const auto lock = Lock{write_lock_};
    const auto it = unit_workflow_map_.find(accountID);

    if (unit_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::ListByState(
    otx::client::PaymentWorkflowType type,
    otx::client::PaymentWorkflowState state) const
    -> PaymentWorkflows::Workflows
{
    const auto lock = Lock{write_lock_};
    const auto it = state_workflow_map_.find(State{type, state});

    if (state_workflow_map_.end() == it) { return {}; }

    return it->second;
}

auto PaymentWorkflows::Load(
    const identifier::Generic& id,
    std::shared_ptr<proto::PaymentWorkflow>& output,
    ErrorReporting checking) const -> bool
{
    UnallocatedCString alias;

    return load_proto<proto::PaymentWorkflow>(id, output, alias, checking);
}

auto PaymentWorkflows::LookupBySource(const identifier::Generic& sourceID) const
    -> identifier::Generic
{
    const auto lock = Lock{write_lock_};
    const auto it = item_workflow_map_.find(sourceID);

    if (item_workflow_map_.end() == it) { return {}; }

    return it->second;
}

void PaymentWorkflows::reindex(
    const Lock& lock,
    const identifier::Generic& workflowID,
    const otx::client::PaymentWorkflowType type,
    const otx::client::PaymentWorkflowState newState,
    otx::client::PaymentWorkflowState& state)
{
    assert_true(verify_write_lock(lock));
    assert_false(workflowID.empty());

    const State oldKey{type, state};
    auto& oldSet = state_workflow_map_[oldKey];
    oldSet.erase(workflowID);

    if (0 == oldSet.size()) { state_workflow_map_.erase(oldKey); }

    state = newState;

    assert_true(otx::client::PaymentWorkflowType::Error != type);
    assert_true(otx::client::PaymentWorkflowState::Error != state);

    const State newKey{type, newState};
    state_workflow_map_[newKey].emplace(workflowID);
}

auto PaymentWorkflows::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();
        LogAbort()().Abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

auto PaymentWorkflows::serialize() const -> proto::StoragePaymentWorkflows
{
    proto::StoragePaymentWorkflows serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                item.first, item.second, *serialized.add_workflow());
        }
    }

    for (const auto& [item, workflow] : item_workflow_map_) {
        auto& newIndex = *serialized.add_items();
        newIndex.set_version(index_version_);
        newIndex.set_workflow(workflow.asBase58(crypto_));
        newIndex.set_item(item.asBase58(crypto_));
    }

    for (const auto& [account, workflowSet] : account_workflow_map_) {
        assert_false(account.empty());

        for (const auto& workflow : workflowSet) {
            assert_false(workflow.empty());

            auto& newAccount = *serialized.add_accounts();
            newAccount.set_version(index_version_);
            newAccount.set_workflow(workflow.asBase58(crypto_));
            newAccount.set_item(account.asBase58(crypto_));
        }
    }

    for (const auto& [unit, workflowSet] : unit_workflow_map_) {
        assert_false(unit.empty());

        for (const auto& workflow : workflowSet) {
            assert_false(workflow.empty());

            auto& newUnit = *serialized.add_units();
            newUnit.set_version(index_version_);
            newUnit.set_workflow(workflow.asBase58(crypto_));
            newUnit.set_item(unit.asBase58(crypto_));
        }
    }

    for (const auto& [workflow, stateTuple] : workflow_state_map_) {
        assert_false(workflow.empty());

        const auto& [type, state] = stateTuple;

        assert_true(otx::client::PaymentWorkflowType::Error != type);
        assert_true(otx::client::PaymentWorkflowState::Error != state);

        auto& newIndex = *serialized.add_types();
        newIndex.set_version(type_version_);
        newIndex.set_workflow(workflow.asBase58(crypto_));
        newIndex.set_type(translate(type));
        newIndex.set_state(translate(state));
    }

    for (const auto& archived : archived_) {
        assert_false(archived.empty());

        serialized.add_archived(archived.asBase58(crypto_));
    }

    return serialized;
}

auto PaymentWorkflows::Store(
    const proto::PaymentWorkflow& data,
    UnallocatedCString& plaintext) -> bool
{
    const auto lock = Lock{write_lock_};
    const UnallocatedCString alias;
    const auto id = factory_.IdentifierFromBase58(data.id());
    delete_by_value(id);

    for (const auto& source : data.source()) {
        item_workflow_map_.emplace(
            factory_.IdentifierFromBase58(source.id()), id);
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
        account_workflow_map_[factory_.AccountIDFromBase58(account)].emplace(
            id);
    }

    for (const auto& unit : data.unit()) {
        unit_workflow_map_[factory_.UnitIDFromBase58(unit)].emplace(id);
    }

    return store_proto(lock, data, id, alias, plaintext);
}

auto PaymentWorkflows::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        case 3u:
        default: {
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
