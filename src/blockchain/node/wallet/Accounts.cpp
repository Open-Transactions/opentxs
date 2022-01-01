// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Accounts.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <map>
#include <random>
#include <set>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/Account.hpp"
#include "blockchain/node/wallet/NotificationStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/HDPath.pb.h"
#include "util/Gatekeeper.hpp"
#include "util/JobCounter.hpp"

namespace opentxs::blockchain::node::wallet
{
struct Accounts::Imp {
    auto Add(const identifier::Nym& nym) noexcept -> bool
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return false; }

        auto [it, added] = map_.try_emplace(
            nym,
            api_,
            api_.Crypto().Blockchain().Account(nym, chain_),
            node_,
            parent_,
            db_,
            filter_type_,
            job_counter_.Allocate(),
            task_finished_);

        if (added) {
            LogConsole()("Initializing ")(DisplayString(chain_))(
                " wallet for ")(nym)
                .Flush();
        }

        index_nym(nym);

        return added;
    }
    auto Add(const zmq::Frame& message) noexcept -> bool
    {
        const auto id = api_.Factory().NymID(message);

        if (0 == id->size()) { return false; }

        return Add(id);
    }
    auto BlockAvailable(const block::Hash& block) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each([&](auto& a) { a.ProcessBlockAvailable(block); });
    }
    auto finish_background_tasks() noexcept -> void
    {
        for_each([](auto& a) { a.FinishBackgroundTasks(); });
    }
    auto Init() noexcept -> void
    {
        for (const auto& id : api_.Wallet().LocalNyms()) { Add(id); }
    }
    auto Mempool(
        std::shared_ptr<const block::bitcoin::Transaction>&& tx) noexcept
        -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each([&](auto& a) { a.ProcessMempool(tx); });
    }
    auto NewFilter(const block::Position& tip) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each([&](auto& a) { a.ProcessNewFilter(tip); });
    }
    auto NewKey() noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each([](auto& a) { a.ProcessKey(); });
    }
    auto Reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return false; }

        auto output{false};

        for_each([&](auto& a) {
            output |= a.ProcessReorg(headerOracleLock, tx, errors, parent);
        });

        return output;
    }
    auto shutdown() noexcept -> void
    {
        gatekeeper_.shutdown();
        for_each([&](auto& a) { a.Shutdown(); });
        payment_codes_.clear();
        map_.clear();
    }
    auto state_machine(bool enabled) noexcept -> bool
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return false; }

        auto output{false};

        for (auto& [code, account] : payment_codes_) {
            output |= account.ProcessStateMachine(enabled);
        }

        for (auto& [nym, account] : map_) {
            output |= account.ProcessStateMachine(enabled);
        }

        return output;
    }
    auto TaskComplete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void
    {
        auto ticket = gatekeeper_.get();

        if (ticket) { return; }

        for_each([&](auto& a) { a.ProcessTaskComplete(id, type, enabled); });
    }

    Imp(const api::Session& api,
        const node::internal::Network& node,
        Accounts& parent,
        const node::internal::WalletDatabase& db,
        const Type chain,
        const std::function<void(const Identifier&, const char*)>&
            taskFinished) noexcept
        : api_(api)
        , node_(node)
        , parent_(parent)
        , db_(db)
        , task_finished_(taskFinished)
        , chain_(chain)
        , filter_type_(node_.FilterOracleInternal().DefaultType())
        , rng_(std::random_device{}())
        , job_counter_()
        , map_()
        , pc_counter_(job_counter_.Allocate())
        , payment_codes_()
        , gatekeeper_()
    {
    }

    ~Imp() { shutdown(); }

private:
    using AccountMap = std::pmr::map<OTNymID, wallet::Account>;
    using PCMap = std::pmr::map<OTIdentifier, NotificationStateData>;

    const api::Session& api_;
    const node::internal::Network& node_;
    Accounts& parent_;
    const node::internal::WalletDatabase& db_;
    const std::function<void(const Identifier&, const char*)>& task_finished_;
    const Type chain_;
    const filter::Type filter_type_;
    std::default_random_engine rng_;
    JobCounter job_counter_;
    AccountMap map_;
    Outstanding pc_counter_;
    PCMap payment_codes_;
    Gatekeeper gatekeeper_;

    auto for_each(std::function<void(Actor&)> action) noexcept -> void
    {
        if (!action) { return; }

        auto actors = std::pmr::vector<Actor*>{};

        for (auto& [nym, account] : map_) { actors.emplace_back(&account); }

        for (auto& [code, account] : payment_codes_) {
            actors.emplace_back(&account);
        }

        std::shuffle(actors.begin(), actors.end(), rng_);

        for (auto* actor : actors) { action(*actor); }
    }

    auto index_nym(const identifier::Nym& id) noexcept -> void
    {
        const auto pNym = api_.Wallet().Nym(id);

        OT_ASSERT(pNym);

        const auto& nym = *pNym;
        auto code = api_.Factory().PaymentCode(nym.PaymentCode());

        if (3 > code.Version()) { return; }

        LogConsole()("Initializing payment code ")(code.asBase58())(" on ")(
            DisplayString(chain_))
            .Flush();
        auto accountID =
            NotificationStateData::calculate_id(api_, chain_, code);
        payment_codes_.try_emplace(
            std::move(accountID),
            api_,
            node_,
            parent_,
            db_,
            task_finished_,
            pc_counter_,
            filter_type_,
            chain_,
            id,
            std::move(code),
            [&] {
                auto out = proto::HDPath{};
                nym.PaymentCodePath(out);

                return out;
            }());
    }
};

Accounts::Accounts(
    const api::Session& api,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const Type chain,
    const std::function<void(const Identifier&, const char*)>&
        taskFinished) noexcept
    : imp_(std::make_unique<Imp>(api, node, *this, db, chain, taskFinished))
{
    imp_->Init();
}

auto Accounts::FinishBackgroundTasks() noexcept -> void
{
    return imp_->finish_background_tasks();
}

auto Accounts::ProcessBlockAvailable(const block::Hash& block) noexcept -> void
{
    imp_->BlockAvailable(block);
}

auto Accounts::ProcessKey() noexcept -> void { imp_->NewKey(); }

auto Accounts::ProcessMempool(
    std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void
{
    imp_->Mempool(std::move(tx));
}

auto Accounts::ProcessNewFilter(const block::Position& tip) noexcept -> void
{
    imp_->NewFilter(tip);
}

auto Accounts::ProcessNym(const identifier::Nym& nym) noexcept -> bool
{
    return imp_->Add(nym);
}

auto Accounts::ProcessNym(const zmq::Frame& message) noexcept -> bool
{
    return imp_->Add(message);
}

auto Accounts::ProcessReorg(
    const Lock& headerOracleLock,
    storage::lmdb::LMDB::Transaction& tx,
    std::atomic_int& errors,
    const block::Position& parent) noexcept -> bool
{
    return imp_->Reorg(headerOracleLock, tx, errors, parent);
}

auto Accounts::ProcessStateMachine(bool enabled) noexcept -> bool
{
    return imp_->state_machine(enabled);
}

auto Accounts::ProcessTaskComplete(
    const Identifier& id,
    const char* type,
    bool enabled) noexcept -> void
{
    imp_->TaskComplete(id, type, enabled);
}

auto Accounts::Shutdown() noexcept -> void { imp_->shutdown(); }

Accounts::~Accounts() { imp_->shutdown(); }
}  // namespace opentxs::blockchain::node::wallet
