// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/wallet/OutputCache.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <chrono>  // IWYU pragma: keep
#include <cstring>
#include <iosfwd>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database::wallet
{
auto all_states() noexcept -> const States&
{
    using State = node::TxoState;
    static const auto data = States{
        State::UnconfirmedNew,
        State::UnconfirmedSpend,
        State::ConfirmedNew,
        State::ConfirmedSpend,
        State::OrphanedNew,
        State::OrphanedSpend,
        State::Immature,
    };

    return data;
}

template <typename MapKeyType, typename DBKeyType, typename MapType>
auto OutputCache::load_output_index(
    const Table table,
    const MapKeyType& key,
    const DBKeyType dbKey,
    const char* indexName,
    const UnallocatedCString& keyName,
    MapType& map) noexcept -> Outpoints&
{
    if (auto it = map.find(key); map.end() != it) { return it->second; }

    auto [row, added] = map.try_emplace(key, Outpoints{});

    OT_ASSERT(added);

    auto& set = row->second;
    set.reserve(reserve_);
#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading ")(
        indexName)(" index for ")(keyName)(" from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    lmdb_.Load(
        table,
        dbKey,
        [&](const auto bytes) { set.emplace(bytes); },
        Mode::Multiple);
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())("loaded ")(set.size())(" items for ")(keyName)
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return set;
}
}  // namespace opentxs::blockchain::database::wallet

namespace opentxs::blockchain::database::wallet
{
const Outpoints OutputCache::empty_outputs_{};
const Nyms OutputCache::empty_nyms_{};

OutputCache::OutputCache(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain,
    const block::Position& blank) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , chain_(chain)
    , blank_(blank)
    , position_lock_()
    , position_()
    , output_lock_()
    , outputs_()
    , account_lock_()
    , accounts_()
    , key_lock_()
    , keys_()
    , nym_lock_()
    , nyms_()
    , nym_list_()
    , positions_lock_()
    , positions_()
    , state_lock_()
    , states_()
    , subchain_lock_()
    , subchains_()
{
    outputs_.reserve(reserve_);
    keys_.reserve(reserve_);
    positions_.reserve(reserve_);
}

auto OutputCache::AddOutput(
    const eLock&,
    const block::Outpoint& id,
    MDB_txn* tx,
    std::unique_ptr<block::bitcoin::Output> pOutput) noexcept -> bool
{
    if (write_output(id, *pOutput, tx)) {
        outputs_.try_emplace(id, std::move(pOutput));

        return true;
    }

    return false;
}

auto OutputCache::AddToAccount(
    const eLock&,
    const AccountID& id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for account ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto& set = load_output_index(
            wallet::accounts_, id, id.Bytes(), "account", id.str(), accounts_);
        auto rc = lmdb_.Store(wallet::accounts_, id.Bytes(), output.Bytes(), tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update account index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToKey(
    const eLock&,
    const crypto::Key& id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for key ")(opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = preimage(id);

    try {
        auto& set = load_output_index(
            wallet::keys_, id, reader(key), "key", opentxs::print(id), keys_);
        auto rc =
            lmdb_.Store(wallet::keys_, reader(key), output.Bytes(), tx).first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToNym(
    const eLock&,
    const identifier::Nym& id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for nym ")(id.str()).Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    OT_ASSERT(false == id.empty());

    try {
        auto& index = load_output_index(
            wallet::nyms_, id, id.Bytes(), "nym", id.str(), nyms_);
        auto& list = load_nyms();
        auto rc =
            lmdb_.Store(wallet::nyms_, id.Bytes(), output.Bytes(), tx).first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update nym index"};
        }

        index.emplace(output);
        list.emplace(id);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToPosition(
    const eLock&,
    const block::Position& id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for position ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = db::Position{id};

    try {
        auto& set = load_output_index(
            wallet::positions_,
            id,
            reader(key.data_),
            "position",
            opentxs::print(id),
            positions_);
        auto rc =
            lmdb_
                .Store(
                    wallet::positions_, reader(key.data_), output.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToState(
    const eLock&,
    const node::TxoState id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for state ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto& set = load_output_index(
            wallet::states_,
            id,
            static_cast<std::size_t>(id),
            "state",
            UnallocatedCString{print(id)},
            states_);
        auto rc = lmdb_
                      .Store(
                          wallet::states_,
                          static_cast<std::size_t>(id),
                          output.Bytes(),
                          tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);
#if defined OPENTXS_DETAILED_DEBUG
        LogTrace()(OT_PRETTY_CLASS())("output ")(output.str())(
            " added to index for state ")(opentxs::print(id))
            .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToSubchain(
    const eLock&,
    const SubchainID& id,
    const block::Outpoint& output,
    MDB_txn* tx) noexcept -> bool
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for subchain ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto& set = load_output_index(
            wallet::subchains_,
            id,
            id.Bytes(),
            "subchain",
            id.str(),
            subchains_);
        auto rc =
            lmdb_.Store(wallet::subchains_, id.Bytes(), output.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update subchain index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::ChangePosition(
    const eLock& lock,
    const block::Position& oldPosition,
    const block::Position& newPosition,
    const block::Outpoint& id,
    MDB_txn* tx) noexcept -> bool
{
    try {
        GetPosition(lock, oldPosition);
        GetPosition(lock, newPosition);
        const auto oldP = db::Position{oldPosition};
        const auto newP = db::Position{newPosition};

        if (lmdb_.Exists(wallet::positions_, reader(oldP.data_), id.Bytes())) {
            auto rc = lmdb_.Delete(
                wallet::positions_, reader(oldP.data_), id.Bytes(), tx);

            if (false == rc) {
                throw std::runtime_error{"Failed to remove old position index"};
            }
        } else {
            LogError()(OT_PRETTY_CLASS())("Warning: position index for ")(
                id.str())(" already removed")
                .Flush();
        }

        auto rc =
            lmdb_.Store(wallet::positions_, reader(newP.data_), id.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to add position state index"};
        }

        auto& from = positions_[oldPosition];
        auto& to = positions_[newPosition];
        from.erase(id);
        to.emplace(id);

        if (0u == from.size()) { positions_.erase(oldPosition); }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::ChangeState(
    const eLock& lock,
    const node::TxoState oldState,
    const node::TxoState newState,
    const block::Outpoint& id,
    MDB_txn* tx) noexcept -> bool
{
    try {
        for (const auto state : all_states()) { GetState(lock, state); }

        auto deleted = UnallocatedVector<node::TxoState>{};

        for (const auto state : all_states()) {
            if (lmdb_.Delete(
                    wallet::states_,
                    static_cast<std::size_t>(state),
                    id.Bytes(),
                    tx)) {
                deleted.emplace_back(state);
            }
        }

        if ((0u == deleted.size()) || (oldState != deleted.front())) {
            LogError()(OT_PRETTY_CLASS())("Warning: state index for ")(
                id.str())(" did not match expected value")
                .Flush();
        }

        if (1u != deleted.size()) {
            LogError()(OT_PRETTY_CLASS())("Warning: output ")(id.str())(
                " found in multiple state indices")
                .Flush();
        }

        auto rc = lmdb_
                      .Store(
                          wallet::states_,
                          static_cast<std::size_t>(newState),
                          id.Bytes(),
                          tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to add new state index"};
        }

        for (const auto& state : all_states()) {
            if (auto it = states_.find(state); states_.end() != it) {
                auto& from = it->second;
                from.erase(id);

                if (0u == from.size()) { states_.erase(it); }
            }
        }

        auto& to = states_[newState];
        to.emplace(id);

        return rc;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::Clear(const eLock&) noexcept -> void
{
    position_ = std::nullopt;
    nym_list_ = std::nullopt;
    outputs_.clear();
    accounts_.clear();
    keys_.clear();
    nyms_.clear();
    positions_.clear();
    states_.clear();
    subchains_.clear();
}

auto OutputCache::GetAccount(const sLock&, const AccountID& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for account ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto lock = Lock{account_lock_};

        return load_output_index(
            wallet::accounts_, id, id.Bytes(), "account", id.str(), accounts_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetAccount(const eLock&, const AccountID& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for account ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        return load_output_index(
            wallet::accounts_, id, id.Bytes(), "account", id.str(), accounts_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetKey(const sLock&, const crypto::Key& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for key ")(opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = preimage(id);

    try {
        auto lock = Lock{key_lock_};

        return load_output_index(
            wallet::keys_, id, reader(key), "key", opentxs::print(id), keys_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetKey(const eLock&, const crypto::Key& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for key ")(opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = preimage(id);

    try {
        return load_output_index(
            wallet::keys_, id, reader(key), "key", opentxs::print(id), keys_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetHeight() noexcept -> block::Height
{
    auto lock = Lock{position_lock_};

    return get_position().Height();
}

auto OutputCache::GetNym(const sLock&, const identifier::Nym& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for nym ")(id.str()).Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto lock = Lock{nym_lock_};

        return load_output_index(
            wallet::nyms_, id, id.Bytes(), "nym", id.str(), nyms_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetNym(const eLock&, const identifier::Nym& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for nym ")(id.str()).Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        return load_output_index(
            wallet::nyms_, id, id.Bytes(), "nym", id.str(), nyms_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetNyms() noexcept -> const Nyms&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading nym list").Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto lock = Lock{nym_lock_};

        return load_nyms();
    } catch (...) {

        return empty_nyms_;
    }
}

auto OutputCache::GetNyms(const eLock&) noexcept -> const Nyms&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading nym list").Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {

        return load_nyms();
    } catch (...) {

        return empty_nyms_;
    }
}

auto OutputCache::GetOutput(const sLock&, const block::Outpoint& id) noexcept(
    false) -> const block::bitcoin::internal::Output&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading output ")(id.str()).Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    auto lock = Lock{output_lock_};

    return load_output(id);
}

auto OutputCache::GetOutput(const eLock&, const block::Outpoint& id) noexcept(
    false) -> block::bitcoin::internal::Output&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading output ")(id.str()).Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return load_output(id);
}

auto OutputCache::GetOutput(
    const eLock& lock,
    const SubchainID& subchain,
    const block::Outpoint& id) noexcept(false)
    -> block::bitcoin::internal::Output&
{
    const auto& relevant = GetSubchain(lock, subchain);

    if (0u == relevant.count(id)) {
        throw std::out_of_range{"outpoint not found in this subchain"};
    }

    return GetOutput(lock, id);
}

auto OutputCache::GetPosition(const eLock&) noexcept -> const db::Position&
{
    return get_position();
}

auto OutputCache::GetPosition(const sLock&, const block::Position& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for position ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = db::Position{id};

    try {
        auto lock = Lock{positions_lock_};

        return load_output_index(
            wallet::positions_,
            id,
            reader(key.data_),
            "position",
            opentxs::print(id),
            positions_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetPosition(const eLock&, const block::Position& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for position ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto key = db::Position{id};

    try {

        return load_output_index(
            wallet::positions_,
            id,
            reader(key.data_),
            "position",
            opentxs::print(id),
            positions_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::get_position() noexcept -> const db::Position&
{
    if (position_.has_value()) { return position_.value(); }

    load_position();

    if (position_.has_value()) { return position_.value(); }

    static const auto null = db::Position{blank_};

    return null;
}

auto OutputCache::GetState(const sLock&, const node::TxoState id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for state ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto lock = Lock{state_lock_};

        return load_output_index(
            wallet::states_,
            id,
            static_cast<std::size_t>(id),
            "state",
            UnallocatedCString{print(id)},
            states_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetState(const eLock&, const node::TxoState id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for state ")(
        opentxs::print(id))
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        return load_output_index(
            wallet::states_,
            id,
            static_cast<std::size_t>(id),
            "state",
            UnallocatedCString{print(id)},
            states_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetSubchain(const sLock&, const SubchainID& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for subchain ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        auto lock = Lock{subchain_lock_};

        return load_output_index(
            wallet::subchains_,
            id,
            id.Bytes(),
            "subchain",
            id.str(),
            subchains_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::GetSubchain(const eLock&, const SubchainID& id) noexcept
    -> const Outpoints&
{
#if defined OPENTXS_DETAILED_DEBUG
    LogInsane()(OT_PRETTY_CLASS())("loading index for subchain ")(id.str())
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    try {
        return load_output_index(
            wallet::subchains_,
            id,
            id.Bytes(),
            "subchain",
            id.str(),
            subchains_);
    } catch (...) {

        return empty_outputs_;
    }
}

auto OutputCache::load_nyms() noexcept -> Nyms&
{
    if (nym_list_.has_value()) { return nym_list_.value(); }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading nym list from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    auto& set = nym_list_.emplace();
    auto tested = UnallocatedSet<ReadView>{};
    lmdb_.Read(
        wallet::nyms_,
        [&](const auto key, const auto bytes) {
            if (0u == tested.count(key)) {
                set.emplace([&] {
                    auto out = api_.Factory().NymID();
                    out->Assign(key);

                    return out;
                }());
                tested.emplace(key);
            }

            return true;
        },
        Dir::Forward);
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
    LogTrace()(OT_PRETTY_CLASS())(set.size())(" nyms found in database")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    return nym_list_.value();
}

auto OutputCache::load_output(const block::Outpoint& id) noexcept(false)
    -> block::bitcoin::internal::Output&
{
    auto it = outputs_.find(id);

    if (outputs_.end() != it) {
        auto& out = it->second->Internal();

        OT_ASSERT(0 < out.Keys().size());

        return out;
    }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading output ")(id.str())(
        " from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    lmdb_.Load(wallet::outputs_, id.Bytes(), [&](const auto bytes) {
        const auto proto =
            proto::Factory<proto::BlockchainTransactionOutput>(bytes);
        auto pOutput = factory::BitcoinTransactionOutput(api_, chain_, proto);

        if (pOutput) {
            auto [row, added] = outputs_.try_emplace(id, std::move(pOutput));

            OT_ASSERT(added);

            it = row;
        }
    });
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG

    if (outputs_.end() != it) {
#if defined OPENTXS_DETAILED_DEBUG
        LogTrace()(OT_PRETTY_CLASS())("output ")(id.str())(" found in database")
            .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
        auto& out = it->second->Internal();

        OT_ASSERT(0 < out.Keys().size());

        return out;
    }

#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("output ")(id.str())(" not found in database")
        .Flush();
#endif  // defined OPENTXS_DETAILED_DEBUG
    const auto error = UnallocatedCString{"output "} + id.str() + " not found";

    throw std::out_of_range{error};
}

auto OutputCache::load_position() noexcept -> void
{
    position_ = std::nullopt;
#if defined OPENTXS_DETAILED_DEBUG
    LogTrace()(OT_PRETTY_CLASS())("cache miss, loading position from database")
        .Flush();
    const auto start = Clock::now();
#endif  // defined OPENTXS_DETAILED_DEBUG
    lmdb_.Load(
        wallet::output_config_,
        tsv(database::Key::WalletPosition),
        [&](const auto bytes) { position_.emplace(bytes); });
#if defined OPENTXS_DETAILED_DEBUG
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start);
    LogTrace()(OT_PRETTY_CLASS())("database query finished in ")(
        elapsed.count())(" microseconds")
        .Flush();

    if (position_.has_value()) {
        LogTrace()(OT_PRETTY_CLASS())("position found").Flush();
    } else {
        LogTrace()(OT_PRETTY_CLASS())("position not available").Flush();
    }
#endif  // defined OPENTXS_DETAILED_DEBUG
}

auto OutputCache::Print(const eLock&) const noexcept -> void
{
    // TODO read from database instead of cache
    struct Output {
        std::stringstream text_{};
        Amount total_{};
    };
    auto output = UnallocatedMap<node::TxoState, Output>{};

    const auto& definition = blockchain::GetDefinition(chain_);

    for (const auto& data : outputs_) {
        const auto& outpoint = data.first;
        const auto& item = data.second->Internal();
        auto& out = output[item.State()];
        out.text_ << "\n * " << outpoint.str() << ' ';
        out.text_ << " value: " << definition.Format(item.Value());
        out.total_ += item.Value();
        const auto& script = item.Script();
        out.text_ << ", type: ";
        using Pattern = block::bitcoin::Script::Pattern;

        switch (script.Type()) {
            case Pattern::PayToMultisig: {
                out.text_ << "P2MS";
            } break;
            case Pattern::PayToPubkey: {
                out.text_ << "P2PK";
            } break;
            case Pattern::PayToPubkeyHash: {
                out.text_ << "P2PKH";
            } break;
            case Pattern::PayToScriptHash: {
                out.text_ << "P2SH";
            } break;
            default: {
                out.text_ << "unknown";
            }
        }

        out.text_ << ", state: " << print(item.State());
    }

    const auto& unconfirmed = output[node::TxoState::UnconfirmedNew];
    const auto& confirmed = output[node::TxoState::ConfirmedNew];
    const auto& pending = output[node::TxoState::UnconfirmedSpend];
    const auto& spent = output[node::TxoState::ConfirmedSpend];
    const auto& orphan = output[node::TxoState::OrphanedNew];
    const auto& outgoingOrphan = output[node::TxoState::OrphanedSpend];
    const auto& immature = output[node::TxoState::Immature];
    auto& log = LogConsole();
    log(OT_PRETTY_CLASS())("Instance ")(api_.Instance())(
        " TXO database contents:")
        .Flush();
    log(OT_PRETTY_CLASS())("Unconfirmed available value: ")(unconfirmed.total_)(
        unconfirmed.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Confirmed available value: ")(confirmed.total_)(
        confirmed.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Unconfirmed spent value: ")(pending.total_)(
        pending.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Confirmed spent value: ")(spent.total_)(
        spent.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Orphaned incoming value: ")(orphan.total_)(
        orphan.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Orphaned spend value: ")(outgoingOrphan.total_)(
        outgoingOrphan.text_.str())
        .Flush();
    log(OT_PRETTY_CLASS())("Immature value: ")(immature.total_)(
        immature.text_.str())
        .Flush();

    log(OT_PRETTY_CLASS())("Outputs by block:\n");

    for (const auto& [position, outputs] : positions_) {
        const auto& [height, hash] = position;
        log("  * block ")(hash->asHex())(" at height ")(height)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by nym:\n");

    for (const auto& [id, outputs] : nyms_) {
        log("  * ")(id->str())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by subaccount:\n");

    for (const auto& [id, outputs] : accounts_) {
        log("  * ")(id->str())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by subchain:\n");

    for (const auto& [id, outputs] : subchains_) {
        log("  * ")(id->str())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by key:\n");

    for (const auto& [key, outputs] : keys_) {
        log("  * ")(opentxs::print(key))("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by state:\n");

    for (const auto& [state, outputs] : states_) {
        log("  * ")(print(state))("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Generation outputs:\n");
    lmdb_.Read(
        generation_,
        [&](const auto key, const auto value) -> bool {
            const auto height = [&] {
                auto out = block::Height{};

                OT_ASSERT(sizeof(out) == key.size());

                std::memcpy(&out, key.data(), key.size());

                return out;
            }();
            const auto outpoint = block::Outpoint{value};
            log("  * height ")(height)(", ")(outpoint.str())("\n");

            return true;
        },
        Dir::Forward);

    log.Flush();
}

auto OutputCache::UpdateOutput(
    const eLock&,
    const block::Outpoint& id,
    const block::bitcoin::Output& output,
    MDB_txn* tx) noexcept -> bool
{
    return write_output(id, output, tx);
}

auto OutputCache::UpdatePosition(
    const eLock&,
    const block::Position& pos,
    MDB_txn* tx) noexcept -> bool
{
    try {
        const auto serialized = db::Position{pos};
        const auto rc = lmdb_
                            .Store(
                                wallet::output_config_,
                                tsv(database::Key::WalletPosition),
                                reader(serialized.data_),
                                tx)
                            .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update wallet position"};
        }

        position_.emplace(pos);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::write_output(
    const block::Outpoint& id,
    const block::bitcoin::Output& output,
    MDB_txn* tx) noexcept -> bool
{
    try {
        for (const auto& key : output.Keys()) {
            const auto sKey = preimage(key);
            auto rc =
                lmdb_.Store(wallet::keys_, reader(sKey), id.Bytes(), tx).first;

            if (false == rc) {
                throw std::runtime_error{"update to key index"};
            }

            auto& cache = [&]() -> Outpoints& {
                if (auto it = keys_.find(key); keys_.end() != it) {

                    return it->second;
                }

                auto& out = keys_[key];
                out.reserve(reserve_);

                return out;
            }();
            cache.emplace(id);
        }

        const auto serialized = [&] {
            auto out = Space{};
            const auto data = [&] {
                auto proto = block::bitcoin::internal::Output::SerializeType{};
                const auto rc = output.Internal().Serialize(proto);

                if (false == rc) {
                    throw std::runtime_error{"failed to serialize as protobuf"};
                }

                return proto;
            }();

            if (false == proto::write(data, writer(out))) {
                throw std::runtime_error{"failed to serialize as bytes"};
            }

            return out;
        }();

        return lmdb_.Store(wallet::outputs_, id.Bytes(), reader(serialized), tx)
            .first;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

OutputCache::~OutputCache() = default;
}  // namespace opentxs::blockchain::database::wallet
