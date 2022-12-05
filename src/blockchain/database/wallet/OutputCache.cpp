// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::TxoState
// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoState

#include "blockchain/database/wallet/OutputCache.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>  // IWYU pragma: keep
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

#include "blockchain/database/wallet/Position.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

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

template <typename MapKeyType, typename MapType>
auto OutputCache::load_output_index(
    const MapKeyType& key,
    MapType& map) noexcept -> Outpoints&
{
    if (auto it = map.find(key); map.end() != it) { return it->second; }

    auto [row, added] = map.try_emplace(key, Outpoints{});

    OT_ASSERT(added);

    return row->second;
}

template <typename MapKeyType, typename MapType>
auto OutputCache::load_output_index(const MapKeyType& key, MapType& map)
    const noexcept -> const Outpoints&
{
    if (auto it = map.find(key); map.end() != it) { return it->second; }

    return empty_outputs_;
}
}  // namespace opentxs::blockchain::database::wallet

namespace opentxs::blockchain::database::wallet
{
const Outpoints OutputCache::empty_outputs_{};
const Nyms OutputCache::empty_nyms_{};

OutputCache::OutputCache(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain,
    const block::Position& blank) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , chain_(chain)
    , blank_(blank)
    , position_()
    , outputs_()
    , accounts_()
    , keys_()
    , nyms_()
    , nym_list_()
    , positions_()
    , states_()
    , subchains_()
    , populated_(false)
{
    outputs_.reserve(reserve_);
    keys_.reserve(reserve_);
    positions_.reserve(reserve_);
}

auto OutputCache::AddOutput(
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx,
    std::unique_ptr<bitcoin::block::Output> pOutput) noexcept -> bool
{
    if (write_output(id, *pOutput, tx)) {
        outputs_.try_emplace(id, std::move(pOutput));

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to write output").Flush();

        return false;
    }
}

auto OutputCache::AddOutput(
    const block::Outpoint& id,
    const node::TxoState state,
    const block::Position& position,
    const AccountID& account,
    const SubchainID& subchain,
    storage::lmdb::Transaction& tx,
    std::unique_ptr<bitcoin::block::Output> output) noexcept -> bool
{
    OT_ASSERT(false == account.empty());
    OT_ASSERT(false == subchain.empty());

    auto rc = AddOutput(id, tx, std::move(output));

    if (false == rc) { return false; }

    rc &= AddToState(state, id, tx);
    rc &= AddToPosition(position, id, tx);
    rc &= AddToAccount(account, id, tx);
    rc &= AddToSubchain(subchain, id, tx);

    return rc;
}

auto OutputCache::AddToAccount(
    const AccountID& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, accounts_);
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
    const crypto::Key& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));

    const auto key = serialize(id);

    try {
        auto& set = load_output_index(id, keys_);
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
    const identifier::Nym& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));
    OT_ASSERT(false == id.empty());

    try {
        auto& index = load_output_index(id, nyms_);
        auto& list = nym_list_;
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
    const block::Position& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));

    const auto key = db::Position{id};

    try {
        auto& set = load_output_index(id, positions_);
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
    const node::TxoState id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, states_);
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

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToSubchain(
    const SubchainID& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    OT_ASSERT(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, subchains_);
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
    const block::Position& oldPosition,
    const block::Position& newPosition,
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
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
    const node::TxoState oldState,
    const node::TxoState newState,
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        for (const auto state : all_states()) { GetState(state); }

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

auto OutputCache::Clear() noexcept -> void
{
    position_ = std::nullopt;
    nym_list_.clear();
    outputs_.clear();
    accounts_.clear();
    keys_.clear();
    nyms_.clear();
    positions_.clear();
    states_.clear();
    subchains_.clear();
    populated_ = false;
}

auto OutputCache::Exists(const block::Outpoint& id) const noexcept -> bool
{
    return 0 < outputs_.count(id);
}

auto OutputCache::Exists(const SubchainID& subchain, const block::Outpoint& id)
    const noexcept -> bool
{
    if (auto it = subchains_.find(subchain); subchains_.end() != it) {
        const auto& set = it->second;

        return 0 < set.count(id);
    } else {

        return false;
    }
}

auto OutputCache::GetAccount(const AccountID& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, accounts_);
}

auto OutputCache::GetKey(const crypto::Key& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, keys_);
}

auto OutputCache::GetHeight() const noexcept -> block::Height
{
    return get_position().Height();
}

auto OutputCache::GetNym(const identifier::Nym& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, nyms_);
}

auto OutputCache::GetNyms() const noexcept -> const Nyms& { return nym_list_; }

auto OutputCache::GetOutput(const block::Outpoint& id) const noexcept(false)
    -> const bitcoin::block::internal::Output&
{
    return load_output(id);
}

auto OutputCache::GetOutput(const block::Outpoint& id) noexcept(false)
    -> bitcoin::block::internal::Output&
{
    return load_output(id);
}

auto OutputCache::GetOutput(
    const SubchainID& subchain,
    const block::Outpoint& id) noexcept(false)
    -> bitcoin::block::internal::Output&
{
    const auto& relevant = GetSubchain(subchain);

    if (0u == relevant.count(id)) {
        throw std::out_of_range{"outpoint not found in this subchain"};
    }

    return GetOutput(id);
}

auto OutputCache::GetPosition() const noexcept -> const db::Position&
{
    return get_position();
}

auto OutputCache::GetPosition(const block::Position& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, positions_);
}

auto OutputCache::get_position() const noexcept -> const db::Position&
{
    if (position_.has_value()) {

        return position_.value();
    } else {
        static const auto null = db::Position{blank_};

        return null;
    }
}

auto OutputCache::GetState(const node::TxoState id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, states_);
}

auto OutputCache::GetSubchain(const SubchainID& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, subchains_);
}

auto OutputCache::load_output(const block::Outpoint& id) noexcept(false)
    -> bitcoin::block::internal::Output&
{
    auto it = outputs_.find(id);

    if (outputs_.end() != it) {
        auto& out = it->second->Internal();

        OT_ASSERT(0 < out.Keys().size());

        return out;
    }

    const auto error = UnallocatedCString{"output "} + id.str() + " not found";

    throw std::out_of_range{error};
}

auto OutputCache::load_output(const block::Outpoint& id) const noexcept(false)
    -> const bitcoin::block::internal::Output&
{
    return const_cast<OutputCache*>(this)->load_output(id);
}

auto OutputCache::Populate() const noexcept -> void
{
    const_cast<OutputCache*>(this)->populate();
}

auto OutputCache::populate() noexcept -> void
{
    if (populated_) { return; }

    auto outputCount = 0_uz;
    const auto outputs = [&](const auto key, const auto value) {
        ++outputCount;
        outputs_.try_emplace(
            key,
            factory::BitcoinTransactionOutput(
                api_.Crypto().Blockchain(),
                api_.Factory(),
                chain_,
                proto::Factory<proto::BlockchainTransactionOutput>(value)));

        return true;
    };
    const auto accounts = [&](const auto key, const auto value) {
        auto& map = accounts_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto keys = [&](const auto key, const auto value) {
        auto& map = keys_;
        auto& set = map[deserialize(key)];
        set.emplace(value);

        return true;
    };
    const auto nyms = [&](const auto key, const auto value) {
        auto& map = nyms_;
        auto id = [&] {
            auto out = identifier::Nym{};
            out.Assign(key);

            return out;
        }();
        nym_list_.emplace(id);
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto positions = [&](const auto key, const auto value) {
        auto& map = positions_;
        auto& set = map[db::Position{key}.Decode(api_)];
        set.emplace(value);

        return true;
    };
    const auto states = [&](const auto key, const auto value) {
        auto& map = states_;
        auto id = [&] {
            auto out = 0_uz;
            std::memcpy(&out, key.data(), std::min(key.size(), sizeof(out)));

            return static_cast<node::TxoState>(out);
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto subchains = [&](const auto key, const auto value) {
        auto& map = subchains_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    auto tx = lmdb_.TransactionRO();
    static constexpr auto fwd = storage::lmdb::Dir::Forward;
    auto rc = lmdb_.Read(wallet::outputs_, outputs, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::accounts_, accounts, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::keys_, keys, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::nyms_, nyms, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::positions_, positions, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::states_, states, fwd, tx);

    OT_ASSERT(rc);

    rc = lmdb_.Read(wallet::subchains_, subchains, fwd, tx);

    OT_ASSERT(rc);

    if (lmdb_.Exists(
            wallet::output_config_, tsv(database::Key::WalletPosition), tx)) {
        rc = lmdb_.Load(
            wallet::output_config_,
            tsv(database::Key::WalletPosition),
            [&](const auto bytes) { position_.emplace(bytes); },
            tx);

        OT_ASSERT(rc);
    }

    OT_ASSERT(outputs_.size() == outputCount);

    populated_ = true;
}

auto OutputCache::Print() const noexcept -> void
{
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
        using Pattern = bitcoin::block::Script::Pattern;

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
        log("  * block ")(position)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by nym:\n");

    for (const auto& [id, outputs] : nyms_) {
        log("  * ")(id)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by subaccount:\n");

    for (const auto& [id, outputs] : accounts_) {
        log("  * ")(id)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by subchain:\n");

    for (const auto& [id, outputs] : subchains_) {
        log("  * ")(id)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log(OT_PRETTY_CLASS())("Outputs by key:\n");

    for (const auto& [key, outputs] : keys_) {
        log("  * ")(print(key))("\n");

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
    const block::Outpoint& id,
    const bitcoin::block::Output& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    return write_output(id, output, tx);
}

auto OutputCache::UpdatePosition(
    const block::Position& pos,
    storage::lmdb::Transaction& tx) noexcept -> bool
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
    const bitcoin::block::Output& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        for (const auto& key : output.Keys()) {
            const auto sKey = serialize(key);
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
                auto proto = bitcoin::block::internal::Output::SerializeType{};
                const auto rc = output.Internal().Serialize(api_, proto);

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
