// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/Simple.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <compare>
#include <functional>
#include <future>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/P0330.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/blockchain/BlockHeaderListener.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/SyncListener.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

RegtestListener::RegtestListener(const ot::api::session::Client& client)
    : block_listener_(std::make_unique<BlockHeaderListener>(client, "client"))
    , sync_listener_(std::make_unique<SyncListener>(client, "client"))
{
}

RegtestListener::~RegtestListener() = default;

Regtest_fixture_simple::Regtest_fixture_simple()
    : Regtest_fixture_normal(ot_, 0, ot::Options{})
    , users_()
    , user_listeners_()
{
}

auto Regtest_fixture_simple::CreateNym(
    const ot::api::session::Client& api,
    const ot::UnallocatedCString& name,
    const ot::crypto::SeedID& seed,
    int index) noexcept -> const User&
{
    const auto reason = api.Factory().PasswordPrompt(__func__);
    auto [it, added] = users_.try_emplace(
        name,
        api.Crypto().Seed().Words(seed, reason),
        name,
        api.Crypto().Seed().Passphrase(seed, reason));

    EXPECT_TRUE(added);

    auto& user = it->second;

    return user;
}

auto Regtest_fixture_simple::ImportBip39(
    const ot::api::Session& api,
    const ot::UnallocatedCString& words) const noexcept -> ot::crypto::SeedID
{
    using SeedLang = ot::crypto::Language;
    using SeedStyle = ot::crypto::SeedStyle;
    const auto reason = api.Factory().PasswordPrompt(__func__);

    return api.Crypto().Seed().ImportSeed(
        ot_.Factory().SecretFromText(words),
        ot_.Factory().SecretFromText(""),
        SeedStyle::BIP39,
        SeedLang::en,
        reason);
}

auto Regtest_fixture_simple::TransactionGenerator(
    const User& user,
    Height height,
    unsigned count,
    unsigned amount) -> Transaction
{
    using OutputBuilder = ot::blockchain::OutputBuilder;
    using Index = ot::Bip32Index;

    auto output = ot::UnallocatedVector<OutputBuilder>{};
    auto meta = ot::UnallocatedVector<OutpointMetadata>{};
    meta.reserve(count);

    const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};
    static const auto baseAmount = ot::Amount{amount};

    const auto reason = user.api_->Factory().PasswordPrompt(__func__);
    const auto& account = GetHDAccount(user);

    for (auto i = Index{0}; i < Index{count}; ++i) {
        const auto index = account.Reserve(
            Subchain::External, client_1_.Factory().PasswordPrompt(""));
        const auto& element =
            account.BalanceElement(Subchain::External, index.value_or(0));
        const auto& key = element.Key();

        const auto& [bytes, value, pattern] = meta.emplace_back(
            element.PubkeyHash(), baseAmount, Pattern::PayToPubkeyHash);
        output.emplace_back(
            value,
            miner_.Factory().BitcoinScriptP2PKH(test_chain_, key, {}),
            keys);
    }

    auto output_transaction = miner_.Factory().BlockchainTransaction(
        test_chain_, height, output, coinbase_fun_, 2, {});
    const auto& txid = transactions_.emplace_back(output_transaction.ID());

    for (auto i = Index{0}; i < Index{count}; ++i) {
        auto& [bytes, amt, pattern] = meta.at(i);
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), i),
            std::forward_as_tuple(
                std::move(bytes), std::move(amt), std::move(pattern)));
    }

    return output_transaction;
}

auto Regtest_fixture_simple::MineBlocks(
    const Height ancestor,
    const std::size_t count) noexcept -> bool
{
    auto target = ancestor + static_cast<int>(count);
    auto blocks = ot::UnallocatedVector<BlockHeaderListener::Future>{};
    auto wallets = ot::UnallocatedVector<SyncListener::Future>{};
    blocks.reserve(users_.size());
    wallets.reserve(users_.size());

    for (auto& listeners : user_listeners_) {
        blocks.emplace_back(
            listeners.second.block_listener_->GetFuture(target));
        wallets.emplace_back(
            listeners.second.sync_listener_->GetFuture(target));
    }

    auto success = Mine(ancestor, count);

    for (auto& future : blocks) {
        EXPECT_TRUE(
            future.wait_for(wait_time_limit_) == std::future_status::ready);
    }

    for (auto& future : wallets) {
        EXPECT_TRUE(
            future.wait_for(wait_time_limit_) == std::future_status::ready);
    }

    return success;
}

auto Regtest_fixture_simple::MineBlocks(
    const User& user,
    Height ancestor,
    unsigned block_number,
    unsigned transaction_number,
    unsigned amount) noexcept
    -> opentxs::blockchain::protocol::bitcoin::base::block::Header
{

    auto target = ancestor + block_number;
    auto blocks = ot::UnallocatedVector<BlockHeaderListener::Future>{};
    auto wallets = ot::UnallocatedVector<SyncListener::Future>{};
    blocks.reserve(users_.size());
    wallets.reserve(users_.size());

    for (auto& listeners : user_listeners_) {
        blocks.emplace_back(
            listeners.second.block_listener_->GetFuture(target));
        wallets.emplace_back(
            listeners.second.sync_listener_->GetFuture(target));
    }

    Generator gen = [&](Height height) -> Transaction {
        return TransactionGenerator(user, height, transaction_number, amount);
    };
    auto mined_header = MineBlocks(ancestor, block_number, gen, {});

    for (auto& future : blocks) {
        EXPECT_TRUE(
            future.wait_for(wait_time_limit_) == std::future_status::ready);

        const auto [height, hash] = future.get();
        EXPECT_EQ(hash, mined_header.Hash());
    }

    for (auto& future : wallets) {
        EXPECT_TRUE(
            future.wait_for(wait_time_limit_) == std::future_status::ready);
    }

    return mined_header;
}

auto Regtest_fixture_simple::MineBlocks(
    Height ancestor,
    std::size_t block_number,
    const Generator& gen,
    ot::UnallocatedVector<Transaction> extra) noexcept
    -> opentxs::blockchain::protocol::bitcoin::base::block::Header
{
    const auto handle = miner_.Network().Blockchain().GetChain(test_chain_);

    EXPECT_TRUE(handle);

    if (false == handle.IsValid()) { return {}; }

    const auto& network = handle.get();
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader =
        headerOracle.LoadHeader(headerOracle.BestHash(ancestor)).asBitcoin();

    for (auto i = 0_uz; i < block_number; ++i) {
        EXPECT_TRUE(gen);

        auto tx = gen(previousHeader.Height() + 1);

        auto block = miner_.Factory().InternalSession().BitcoinBlock(
            previousHeader,
            std::move(tx),
            previousHeader.nBits(),
            extra,
            previousHeader.Version(),
            [start{ot::Clock::now()}] {
                return (ot::Clock::now() - start) > std::chrono::minutes(2);
            },
            {});

        EXPECT_TRUE(block.IsValid());

        const auto added = network.AddBlock(block);

        EXPECT_TRUE(added);

        previousHeader = block.Header().asBitcoin();
    }

    return previousHeader;
}

auto Regtest_fixture_simple::CreateClient(
    ot::Options client_args,
    int instance,
    const ot::UnallocatedCString& name,
    const ot::UnallocatedCString& words,
    const ot::network::blockchain::Address& address)
    -> std::pair<const User&, bool>
{
    const auto& client = ot_.StartClientSession(client_args, instance);
    const auto start = client.Network().Blockchain().Start(test_chain_);
    const auto handle = client.Network().Blockchain().GetChain(test_chain_);

    opentxs::assert_true(handle);

    const auto& network = handle.get();
    const auto added = network.AddPeer(address);

    auto seed = ImportBip39(client, words);
    const auto& userNym = CreateNym(client, name, seed, instance);

    auto cb = [](User& user) {
        const auto& api = *user.api_;
        const auto& nymID = user.nym_id_;
        const auto reason = api.Factory().PasswordPrompt(__func__);
        api.Crypto().Blockchain().NewHDSubaccount(
            nymID,
            opentxs::blockchain::crypto::HDProtocol::BIP_44,
            test_chain_,
            reason);
    };

    auto& user_no_const = const_cast<User&>(userNym);
    user_no_const.init_custom(client, cb);

    client.UI().Internal().AccountActivity(
        userNym.nym_id_, GetHDAccount(userNym).Parent().AccountID(), []() {});
    client.UI().Internal().AccountList(userNym.nym_id_, []() {});

    const auto [it, listener_added] = user_listeners_.emplace(name, client);

    std::promise<void> promise;
    std::future<void> done = promise.get_future();
    auto cb_connected = [&](auto&& msg, auto& counter) { promise.set_value(); };
    std::atomic_int client_peers;
    ot::OTZMQListenCallback client_cb_(
        ot::network::zeromq::ListenCallback::Factory(
            [&](auto&& msg) { cb_connected(std::move(msg), client_peers); }));
    ot::OTZMQSubscribeSocket client_socket(
        userNym.api_->Network().ZeroMQ().Internal().SubscribeSocket(
            client_cb_));
    if (!client_socket->Start(
            (wait_for_handshake_
                 ? userNym.api_->Endpoints().BlockchainPeer()
                 : userNym.api_->Endpoints().BlockchainPeerConnection())
                .data())) {
        throw std::runtime_error("Error connecting to client1 socket");
    }

    const auto status = done.wait_for(std::chrono::minutes(2));
    const auto future = (std::future_status::ready == status);

    return {userNym, added && start && future && listener_added};
}

auto Regtest_fixture_simple::CloseClient(const ot::UnallocatedCString& name)
    -> void
{
    users_.at(name).api_->Network().Blockchain().Stop(test_chain_);
    users_.erase(name);
    user_listeners_.erase(name);
}

auto Regtest_fixture_simple::GetBalance(const User& user) -> const Amount
{
    const auto& account = GetHDAccount(user);
    const auto& id = account.Parent().AccountID();
    const auto& widget =
        user.api_->UI().Internal().AccountActivity(user.nym_id_, id);
    return widget.Balance();
}

auto Regtest_fixture_simple::GetDisplayBalance(const User& user)
    -> const ot::UnallocatedCString
{
    const auto& account = GetHDAccount(user);
    const auto& id = account.Parent().AccountID();
    const auto& widget =
        user.api_->UI().Internal().AccountActivity(user.nym_id_, id);
    return widget.DisplayBalance();
}

auto Regtest_fixture_simple::GetSyncProgress(const User& user)
    -> const std::pair<int, int>
{
    const auto& account = GetHDAccount(user);
    const auto& id = account.Parent().AccountID();
    const auto& widget =
        user.api_->UI().Internal().AccountActivity(user.nym_id_, id);
    return widget.SyncProgress();
}

auto Regtest_fixture_simple::GetSyncPercentage(const User& user) -> double
{
    const auto& account = GetHDAccount(user);
    const auto& id = account.Parent().AccountID();
    const auto& widget =
        user.api_->UI().Internal().AccountActivity(user.nym_id_, id);
    return widget.SyncPercentage();
}

auto Regtest_fixture_simple::GetHDAccount(const User& user) const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return user.api_->Crypto()
        .Blockchain()
        .Account(user.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_fixture_simple::GetNextBlockchainAddress(const User& user)
    -> const ot::UnallocatedCString
{
    const auto& account = GetHDAccount(user);
    const auto index = account.Reserve(
        Subchain::External, user.api_->Factory().PasswordPrompt(""));
    const auto& element =
        account.BalanceElement(Subchain::External, index.value_or(0));

    return element.Address(opentxs::blockchain::crypto::AddressStyle::P2PKH);
}

auto Regtest_fixture_simple::WaitForSynchro(
    const User& user,
    const Height target,
    const Amount expected_balance) -> void
{
    if (expected_balance == 0) { return; }

    auto begin = std::chrono::steady_clock::now();
    auto now = begin;
    auto end = begin + wait_time_limit_;

    while (now < end) {
        now = std::chrono::steady_clock::now();
        auto progress = GetSyncProgress(user);
        auto balance = GetBalance(user);
        std::ostringstream percentage;
        percentage.precision(2);
        percentage << std::fixed << GetSyncPercentage(user);

        ot::LogConsole()(
            "Waiting for synchronization, balance: " + GetDisplayBalance(user) +
            ", sync percentage: " + percentage.str() + "%, sync progress [" +
            std::to_string(progress.first) + "," +
            std::to_string(progress.second) + "]" +
            ", target height: " + std::to_string(target))
            .Flush();
        if (progress.first == target && progress.second == target &&
            (balance == expected_balance)) {
            ot::LogConsole()(
                "Client synchronized in " +
                std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                                   now - begin)
                                   .count()) +
                " seconds")
                .Flush();
            break;
        }
        ot::Sleep(std::chrono::seconds(5));
    }
}
}  // namespace ottest
