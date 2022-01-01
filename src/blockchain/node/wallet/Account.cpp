// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Account.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/DeterministicStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Iterator.hpp"
#include "util/Gatekeeper.hpp"
#include "util/JobCounter.hpp"

namespace opentxs::blockchain::node::wallet
{
using Subchain = node::internal::WalletDatabase::Subchain;

struct Account::Imp {
    auto block_available(const block::Hash& block) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each_subchain([&](auto& s) { s.ProcessBlockAvailable(block); });
    }
    auto finish_background_tasks() noexcept -> void
    {
        for_each_account([](auto& s) { s.FinishBackgroundTasks(); });
    }
    auto mempool(std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each_subchain([&](auto& s) { s.ProcessMempool(tx); });
    }
    auto new_filter(const block::Position& tip) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each_subchain([&](auto& s) { s.ProcessNewFilter(tip); });
    }
    auto new_key() noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each_subchain([&](auto& s) { s.ProcessKey(); });
    }
    auto reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return false; }

        auto output{false};

        for_each_subchain([&](auto& s) {
            output |= s.ProcessReorg(headerOracleLock, tx, errors, parent);
        });

        return output;
    }
    auto shutdown() noexcept -> void
    {
        gatekeeper_.shutdown();
        for_each_subchain([&](auto& s) { s.Shutdown(); });
        internal_.clear();
        external_.clear();
        outgoing_.clear();
        incoming_.clear();
    }
    auto state_machine(bool enabled) noexcept -> bool
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return false; }

        auto output{false};

        for_each_subchain(
            [&](auto& s) { output |= s.ProcessStateMachine(enabled); });

        return output;
    }
    auto task_complete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each_subchain(
            [&](auto& s) { s.ProcessTaskComplete(id, type, enabled); });
    }

    Imp(const api::Session& api,
        const BalanceTree& ref,
        const node::internal::Network& node,
        Accounts& parent,
        const node::internal::WalletDatabase& db,
        const filter::Type filter,
        Outstanding&& jobs,
        const std::function<void(const Identifier&, const char*)>&
            taskFinished) noexcept
        : api_(api)
        , ref_(ref)
        , node_(node)
        , parent_(parent)
        , db_(db)
        , filter_type_(node_.FilterOracleInternal().DefaultType())
        , task_finished_(taskFinished)
        , rng_(std::random_device{}())
        , internal_()
        , external_()
        , outgoing_()
        , incoming_()
        , jobs_(std::move(jobs))
        , gatekeeper_()
    {
        for (const auto& account : ref_.GetHD()) {
            instantiate(account, Subchain::Internal, internal_);
            instantiate(account, Subchain::External, external_);
        }

        for (const auto& account : ref_.GetPaymentCode()) {
            instantiate(account, Subchain::Outgoing, outgoing_);
            instantiate(account, Subchain::Incoming, incoming_);
        }
    }
    ~Imp() { shutdown(); }

private:
    using Map = std::pmr::map<OTIdentifier, DeterministicStateData>;

    const api::Session& api_;
    const BalanceTree& ref_;
    const node::internal::Network& node_;
    Accounts& parent_;
    const node::internal::WalletDatabase& db_;
    const filter::Type filter_type_;
    const std::function<void(const Identifier&, const char*)>& task_finished_;
    std::default_random_engine rng_;
    Map internal_;
    Map external_;
    Map outgoing_;
    Map incoming_;
    Outstanding jobs_;
    Gatekeeper gatekeeper_;

    template <typename Action>
    auto for_each_account(Action action) noexcept -> void
    {
        for (auto& [id, account] : internal_) { action(account); }

        for (auto& [id, account] : internal_) { action(account); }

        for (auto& [id, account] : outgoing_) { action(account); }

        for (auto& [id, account] : incoming_) { action(account); }
    }
    template <typename Action>
    auto for_each_subchain(Action action) noexcept -> void
    {
        auto actors = std::pmr::vector<Actor*>{};

        for (const auto& account : ref_.GetHD()) {
            actors.emplace_back(&get(account, Subchain::Internal, internal_));
            actors.emplace_back(&get(account, Subchain::External, external_));
        }

        for (const auto& account : ref_.GetPaymentCode()) {
            actors.emplace_back(&get(account, Subchain::Outgoing, outgoing_));
            actors.emplace_back(&get(account, Subchain::Incoming, incoming_));
        }

        std::shuffle(actors.begin(), actors.end(), rng_);

        for (auto* actor : actors) { action(*actor); }
    }
    auto get(
        const crypto::Deterministic& account,
        const Subchain subchain,
        Map& map) noexcept -> DeterministicStateData&
    {
        auto it = map.find(account.ID());

        if (map.end() != it) { return it->second; }

        return instantiate(account, subchain, map);
    }
    auto instantiate(
        const crypto::Deterministic& account,
        const Subchain subchain,
        Map& map) noexcept -> DeterministicStateData&
    {
        auto [it, added] = map.try_emplace(
            account.ID(),
            api_,
            node_,
            parent_,
            db_,
            account,
            task_finished_,
            jobs_,
            filter_type_,
            subchain);

        OT_ASSERT(added);

        return it->second;
    }
};

Account::Account(
    const api::Session& api,
    const BalanceTree& ref,
    const node::internal::Network& node,
    Accounts& parent,
    const node::internal::WalletDatabase& db,
    const filter::Type filter,
    Outstanding&& jobs,
    const std::function<void(const Identifier&, const char*)>&
        taskFinished) noexcept
    : imp_(std::make_unique<Imp>(
          api,
          ref,
          node,
          parent,
          db,
          filter,
          std::move(jobs),
          taskFinished))
{
    OT_ASSERT(imp_);
}

Account::Account(Account&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Account::FinishBackgroundTasks() noexcept -> void
{
    return imp_->finish_background_tasks();
}

auto Account::ProcessBlockAvailable(const block::Hash& block) noexcept -> void
{
    imp_->block_available(block);
}

auto Account::ProcessKey() noexcept -> void { imp_->new_key(); }

auto Account::ProcessMempool(
    std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    imp_->mempool(tx);
}

auto Account::ProcessNewFilter(const block::Position& tip) noexcept -> void
{
    imp_->new_filter(tip);
}

auto Account::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> bool
{
    return imp_->reorg(headerOracleLock, tx, errors, parent);
}

auto Account::ProcessStateMachine(bool enabled) noexcept -> bool
{
    return imp_->state_machine(enabled);
}

auto Account::ProcessTaskComplete(
    const Identifier& id,
    const char* type,
    bool enabled) noexcept -> void
{
    imp_->task_complete(id, type, enabled);
}

auto Account::Shutdown() noexcept -> void { imp_->shutdown(); }

Account::~Account() { imp_->shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
