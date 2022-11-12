// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/TXOs.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "ottest/fixtures/blockchain/TXOState.hpp"
#include "ottest/fixtures/blockchain/regtest/Base.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
struct TXOs::Imp {
    auto AddConfirmed(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
    {
        return add_to_map(tx, index, owner, confirmed_incoming_);
    }
    auto AddGenerated(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        const ot::blockchain::block::Height position) noexcept -> bool
    {
        try {
            const auto& output = tx.Outputs().at(index);
            const auto [it, added] = immature_[position].emplace(
                std::piecewise_construct,
                std::forward_as_tuple(owner.ID()),
                std::forward_as_tuple(tx.ID(), index, output.Value()));

            return added;
        } catch (...) {

            return false;
        }
    }
    auto AddUnconfirmed(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
    {
        return add_to_map(tx, index, owner, unconfirmed_incoming_);
    }
    auto Confirm(const ot::blockchain::block::Txid& txid) noexcept -> bool
    {
        auto confirmed =
            move_txos(txid, unconfirmed_incoming_, confirmed_incoming_);
        confirmed += move_txos(txid, unconfirmed_spent_, confirmed_spent_);

        return 0 < confirmed;
    }
    auto Mature(const ot::blockchain::block::Height pos) noexcept -> bool
    {
        auto output{true};

        for (auto i{immature_.begin()}; i != immature_.end();) {
            auto& [height, outputs] = *i;
            const auto mature =
                (pos - height) >= Regtest_fixture_base::MaturationInterval();

            if (mature) {
                for (const auto& [subaccount, txo] : outputs) {
                    const auto [it, added] =
                        confirmed_incoming_[subaccount].emplace(txo);
                    output &= added;
                }

                i = immature_.erase(i);
            } else {
                break;
            }
        }

        return output;
    }
    auto Orphan(const ot::blockchain::block::Txid& txid) noexcept -> bool
    {
        return true;  // TODO
    }
    auto OrphanGeneration(const ot::blockchain::block::Txid& txid) noexcept
        -> bool
    {
        return true;  // TODO
    }
    auto SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool
    {
        return spend_txo(txo, unconfirmed_spent_);
    }
    auto SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool
    {
        return spend_txo(txo, confirmed_spent_);
    }

    auto Extract(TXOState& output) const noexcept -> void
    {
        using State = ot::blockchain::node::TxoState;
        static constexpr auto all{State::All};
        auto& nym = output.nyms_[user_.nym_id_];
        auto& [wConfirmed, wUnconfirmed] = output.wallet_.balance_;
        auto& [nConfirmed, nUnconfirmed] = nym.nym_.balance_;
        auto& wMap = output.wallet_.data_;
        auto& nMap = nym.nym_.data_;

        for (const auto& [account, txos] : confirmed_incoming_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += value;
                nConfirmed += value;
                aConfirmed += value;
                wUnconfirmed += value;
                nUnconfirmed += value;
                aUnconfirmed += value;
                static constexpr auto state{State::ConfirmedNew};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : unconfirmed_incoming_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += value;
                nUnconfirmed += value;
                aUnconfirmed += value;
                static constexpr auto state{State::UnconfirmedNew};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : confirmed_spent_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::ConfirmedSpend};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [account, txos] : unconfirmed_spent_) {
            auto& aData = nym.accounts_[account];
            auto& [aConfirmed, aUnconfirmed] = aData.balance_;
            auto& aMap = aData.data_;

            for (const auto& [outpoint, value] : txos) {
                wConfirmed += value;
                nConfirmed += value;
                aConfirmed += value;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::UnconfirmedSpend};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }

        for (const auto& [height, data] : immature_) {
            for (const auto& [account, txo] : data) {
                const auto& outpoint = txo.outpoint_;
                auto& aData = nym.accounts_[account];
                auto& [aConfirmed, aUnconfirmed] = aData.balance_;
                auto& aMap = aData.data_;
                wConfirmed += 0;
                nConfirmed += 0;
                aConfirmed += 0;
                wUnconfirmed += 0;
                nUnconfirmed += 0;
                aUnconfirmed += 0;
                static constexpr auto state{State::Immature};
                wMap[state].emplace(outpoint);
                nMap[state].emplace(outpoint);
                aMap[state].emplace(outpoint);
                wMap[all].emplace(outpoint);
                nMap[all].emplace(outpoint);
                aMap[all].emplace(outpoint);
            }
        }
    }

    Imp(const User& owner) noexcept
        : user_(owner)
        , unconfirmed_incoming_()
        , confirmed_incoming_()
        , unconfirmed_spent_()
        , confirmed_spent_()
        , immature_()
    {
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    struct TXO {
        ot::blockchain::block::Outpoint outpoint_;
        ot::blockchain::Amount value_;

        auto operator<(const TXO& rhs) const noexcept -> bool
        {
            if (outpoint_ < rhs.outpoint_) { return true; }

            if (rhs.outpoint_ < outpoint_) { return false; }

            return value_ < rhs.value_;
        }

        TXO(const ot::blockchain::block::Txid& txid,
            const std::size_t index,
            ot::blockchain::Amount value)
        noexcept
            : outpoint_(txid.Bytes(), static_cast<std::uint32_t>(index))
            , value_(value)
        {
        }
        TXO() = delete;
        TXO(const TXO& rhs) = default;
        TXO(TXO&&) = delete;
        auto operator=(const TXO&) -> TXO& = delete;
        auto operator=(TXO&&) -> TXO& = delete;
    };

    using TXOSet = ot::UnallocatedSet<TXO>;
    using Map = ot::UnallocatedMap<ot::identifier::Generic, TXOSet>;
    using Immature = ot::UnallocatedMap<
        ot::blockchain::block::Height,
        ot::UnallocatedSet<std::pair<ot::identifier::Generic, TXO>>>;

    const User& user_;
    Map unconfirmed_incoming_;
    Map confirmed_incoming_;
    Map unconfirmed_spent_;
    Map confirmed_spent_;
    Immature immature_;

    auto add_to_map(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        Map& map) noexcept -> bool
    {
        try {
            const auto& output = tx.Outputs().at(index);
            const auto [it, added] =
                map[owner.ID()].emplace(tx.ID(), index, output.Value());

            return added;
        } catch (...) {

            return false;
        }
    }
    auto move_txo(
        const ot::blockchain::block::Outpoint& target,
        Map& from,
        Map& to) noexcept -> bool
    {
        try {
            auto set = Map::iterator{from.end()};
            auto node = search(target, from, set);
            auto& dest = to[set->first];
            dest.insert(set->second.extract(node));

            return true;
        } catch (...) {

            return false;
        }
    }
    auto move_txos(
        const ot::blockchain::block::Txid& txid,
        Map& from,
        Map& to) noexcept -> std::size_t
    {
        auto toMove =
            ot::UnallocatedVector<std::pair<Map::iterator, TXOSet::iterator>>{};

        for (auto s{from.begin()}; s != from.end(); ++s) {
            auto& set = s->second;

            for (auto t{set.begin()}; t != set.end(); ++t) {
                if (t->outpoint_.Txid() == txid.Bytes()) {
                    toMove.emplace_back(s, t);
                }
            }
        }

        for (auto& [set, node] : toMove) {
            auto& dest = to[set->first];
            dest.insert(set->second.extract(node));
        }

        return toMove.size();
    }
    auto search(
        const ot::blockchain::block::Outpoint& target,
        Map& map,
        Map::iterator& output) noexcept(false) -> TXOSet::iterator
    {
        for (auto s{map.begin()}; s != map.end(); ++s) {
            auto& value = s->second;

            for (auto t{value.begin()}; t != value.end(); ++t) {
                if (t->outpoint_ == target) {
                    output = s;

                    return t;
                }
            }
        }

        throw std::runtime_error{"not found"};
    }
    auto spend_txo(const ot::blockchain::block::Outpoint& txo, Map& to) noexcept
        -> bool
    {
        if (move_txo(txo, confirmed_incoming_, to)) {

            return true;
        } else if (move_txo(txo, unconfirmed_incoming_, to)) {

            return true;
        } else {

            return false;
        }
    }
};
}  // namespace ottest

namespace ottest
{
TXOs::TXOs(const User& owner) noexcept
    : imp_(std::make_unique<Imp>(owner))
{
}

auto TXOs::AddConfirmed(
    const ot::blockchain::bitcoin::block::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
{
    return imp_->AddConfirmed(tx, index, owner);
}

auto TXOs::AddGenerated(
    const ot::blockchain::bitcoin::block::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner,
    const ot::blockchain::block::Height position) noexcept -> bool
{
    return imp_->AddGenerated(tx, index, owner, position);
}

auto TXOs::AddUnconfirmed(
    const ot::blockchain::bitcoin::block::Transaction& tx,
    const std::size_t index,
    const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool
{
    return imp_->AddUnconfirmed(tx, index, owner);
}

auto TXOs::Confirm(const ot::blockchain::block::Txid& transaction) noexcept
    -> bool
{
    return imp_->Confirm(transaction);
}

auto TXOs::Mature(const ot::blockchain::block::Height position) noexcept -> bool
{
    return imp_->Mature(position);
}

auto TXOs::Orphan(const ot::blockchain::block::Txid& transaction) noexcept
    -> bool
{
    return imp_->Orphan(transaction);
}

auto TXOs::OrphanGeneration(
    const ot::blockchain::block::Txid& transaction) noexcept -> bool
{
    return imp_->OrphanGeneration(transaction);
}

auto TXOs::SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
    -> bool
{
    return imp_->SpendUnconfirmed(txo);
}

auto TXOs::SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
    -> bool
{
    return imp_->SpendConfirmed(txo);
}

auto TXOs::Extract(TXOState& output) const noexcept -> void
{
    return imp_->Extract(output);
}

TXOs::~TXOs() = default;
}  // namespace ottest
