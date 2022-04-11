// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Helpers.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"

namespace ottest
{

struct RegtestListener {
    RegtestListener(const ot::api::session::Client& client);

    std::unique_ptr<BlockListener> block_listener;
    std::unique_ptr<WalletListener> wallet_listener;
};

class Regtest_fixture_simple : virtual public Regtest_fixture_single
{
    //const ot::api::Context& ot_;

protected:
    Regtest_fixture_simple();

    using UserIndex = ot::UnallocatedMap<ot::UnallocatedCString, User>;
    using UserListeners = ot::UnallocatedMap<ot::UnallocatedCString, RegtestListener>;

    UserIndex users_;
    UserListeners user_listeners_;
    bool wait_for_handshake_ = true;
    static constexpr auto wait_time_limit_ = std::chrono::minutes(5);
    const unsigned amount_in_transaction_ = 100000;
    const unsigned transaction_in_block_ = 100;

    auto CreateNym(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& name,
        const ot::UnallocatedCString& seed,
        int index) noexcept -> const User&;

    auto ImportBip39(
        const ot::api::Session& api,
        const ot::UnallocatedCString& words) const noexcept
        -> ot::UnallocatedCString;

    auto CreateClient(
        ot::Options client_args,
        int instance,
        const ot::UnallocatedCString& name,
        const ot::UnallocatedCString& words,
        const b::p2p::Address& address)
        -> std::pair<const User&, bool>;

    auto MineBlocks(
        const User& user,
        Height ancestor,
        std::size_t block_number,
        std::size_t transaction_number,
        unsigned amount) noexcept
        -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Header>;

    auto MineBlocks(
        Height ancestor,
        std::size_t block_number,
        const Generator& gen,
        const ot::UnallocatedVector<Transaction>& extra) noexcept
        -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Header>;

    auto MineBlocks(
        const Height ancestor,
        const std::size_t count) noexcept -> bool;

    auto TransactionGenerator(
        const User& user,
        Height height,
        unsigned count,
        unsigned amount)
        -> Transaction;

    auto GetBalance(const User& user) -> const Amount;

    auto GetNextBlockchainAddress(const User& user) -> const ot::UnallocatedCString;

    auto GetHDAccount(const User& user) const noexcept -> const bca::HD&;
};

}