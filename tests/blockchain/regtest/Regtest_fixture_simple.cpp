//
// Created by adam.tkaczyk on 3/30/22.
//

#include "Regtest_fixture_simple.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/api/crypto/Seed.hpp"

namespace ottest
{

RegtestListener::RegtestListener(const ot::api::session::Client& client)
    : block_listener(std::make_unique<BlockListener>(client))
    , wallet_listener(std::make_unique<WalletListener>(client))
{
}

Regtest_fixture_simple::Regtest_fixture_simple()
    : Regtest_fixture_single(ot::Options{}.SetBlockchainStorageLevel(1))
    , users_()
    , user_listeners_()
{
}

auto Regtest_fixture_simple::CreateNym(
    const ot::api::session::Client& api,
    const ot::UnallocatedCString& name,
    const ot::UnallocatedCString& seed,
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
    const ot::UnallocatedCString& words) const noexcept
    -> ot::UnallocatedCString
{
    using SeedLang = ot::crypto::Language;
    using SeedStyle = ot::crypto::SeedStyle;
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto id = api.Crypto().Seed().ImportSeed(
        ot_.Factory().SecretFromText(words),
        ot_.Factory().SecretFromText(""),
        SeedStyle::BIP39,
        SeedLang::en,
        reason);

    return id;
}

auto Regtest_fixture_simple::TransactionGenerator(
    const User& user,
    Height height,
    unsigned count,
    unsigned amount) -> Transaction
{
    using OutputBuilder = ot::api::session::Factory::OutputBuilder;
    using Index = ot::Bip32Index;
    using Subchain = bca::Subchain;

    auto output = ot::UnallocatedVector<OutputBuilder>{};
    auto meta = ot::UnallocatedVector<OutpointMetadata>{};
    meta.reserve(count);

    const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};
    static const auto baseAmount = ot::blockchain::Amount{amount};

    const auto reason = user.api_->Factory().PasswordPrompt(__func__);
    auto& account = GetHDAccount(user);

    for (auto i = Index{0}; i < Index{count}; ++i) {
        const auto index = account.Reserve(
            Subchain::External, client_1_.Factory().PasswordPrompt(""));
        const auto& element =
            account.BalanceElement(Subchain::External, index.value_or(0));
        const auto key = element.Key();

        const auto& [bytes, value, pattern] = meta.emplace_back(
            element.PubkeyHash(), baseAmount, Pattern::PayToPubkeyHash);
        output.emplace_back(
            value,
            miner_.Factory().BitcoinScriptP2PKH(test_chain_, *key),
            keys);
    }

    auto output_transaction = miner_.Factory().BitcoinGenerationTransaction(
        test_chain_, height, std::move(output), coinbase_fun_);

    const auto& txid =
        transactions_.emplace_back(output_transaction->ID()).get();

    for (auto i = Index{0}; i < Index{count}; ++i) {
        auto& [bytes, amount, pattern] = meta.at(i);
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), i),
            std::forward_as_tuple(
                std::move(bytes), std::move(amount), std::move(pattern)));
    }

    return output_transaction;
}

auto Regtest_fixture_simple::MineBlocks(
    const Height ancestor,
    const std::size_t count) noexcept -> bool
{
    auto target = ancestor + static_cast<int>(count);
    auto blocks = ot::UnallocatedVector<BlockListener::Future>{};
    auto wallets = ot::UnallocatedVector<WalletListener::Future>{};
    blocks.reserve(users_.size());
    wallets.reserve(users_.size());

    for (auto& listeners : user_listeners_) {
        blocks.emplace_back(listeners.second.block_listener->GetFuture(target));
        wallets.emplace_back(
            listeners.second.wallet_listener->GetFuture(target));
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
    -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Header>
{

    auto target = ancestor + block_number;
    auto blocks = ot::UnallocatedVector<BlockListener::Future>{};
    auto wallets = ot::UnallocatedVector<WalletListener::Future>{};
    blocks.reserve(users_.size());
    wallets.reserve(users_.size());

    for (auto& listeners : user_listeners_) {
        blocks.emplace_back(listeners.second.block_listener->GetFuture(target));
        wallets.emplace_back(
            listeners.second.wallet_listener->GetFuture(target));
    }

    Generator gen = [&](Height height) -> Transaction {
        return TransactionGenerator(user, height, transaction_number, amount);
    };
    auto mined_header = MineBlocks(ancestor, block_number, gen, {});

    for (auto& future : blocks) {
        EXPECT_TRUE(
            future.wait_for(wait_time_limit_) == std::future_status::ready);

        const auto [height, hash] = future.get();

        EXPECT_EQ(hash, mined_header->Hash());
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
    const ot::UnallocatedVector<Transaction>& extra) noexcept
    -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Header>
{
    const auto& network = miner_.Network().Blockchain().GetChain(test_chain_);
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader =
        headerOracle.LoadHeader(headerOracle.BestHash(ancestor))->as_Bitcoin();

    for (auto i = std::size_t{0u}; i < block_number; ++i) {
        EXPECT_TRUE(gen);

        auto tx = gen(previousHeader->Height() + 1);

        auto block = miner_.Factory().BitcoinBlock(
            *previousHeader,
            tx,
            previousHeader->nBits(),
            extra,
            previousHeader->Version(),
            [start{ot::Clock::now()}] {
                return (ot::Clock::now() - start) > std::chrono::minutes(2);
            });

        EXPECT_TRUE(block);

        const auto added = network.AddBlock(block);

        EXPECT_TRUE(added);

        previousHeader = block->Header().as_Bitcoin();

        EXPECT_TRUE(previousHeader);
    }

    return previousHeader;
}

auto Regtest_fixture_simple::CreateClient(
    ot::Options client_args,
    int instance,
    const ot::UnallocatedCString& name,
    const ot::UnallocatedCString& words,
    const b::p2p::Address& address) -> std::pair<const User&, bool>
{
    auto& client = ot_.StartClientSession(client_args, instance);

    const auto start = client.Network().Blockchain().Start(test_chain_);

    const auto& network = client.Network().Blockchain().GetChain(test_chain_);
    const auto added = network.AddPeer(address);

    auto seed = ImportBip39(client, words);
    auto& user = CreateNym(client, name, seed, instance);

    auto cb = [](User& user) {
        const auto& api = *user.api_;
        const auto& nymID = user.nym_id_.get();
        const auto reason = api.Factory().PasswordPrompt(__func__);
        api.Crypto().Blockchain().NewHDSubaccount(
            nymID,
            opentxs::blockchain::crypto::HDProtocol::BIP_44,
            test_chain_,
            reason);
    };

    auto& user_no_const = const_cast<User&>(user);
    user_no_const.init_custom(client, cb);

    const auto [it, listener_added] = user_listeners_.emplace(name, client);

    std::promise<void> promise;
    std::future<void> done = promise.get_future();
    auto cb_connected = [&](zmq::Message&& msg, std::atomic_int& counter) {
        promise.set_value();
    };
    std::atomic_int client_peers;
    ot::OTZMQListenCallback client_cb_(
        ot::network::zeromq::ListenCallback::Factory(
            [&](auto&& msg) { cb_connected(std::move(msg), client_peers); }));
    ot::OTZMQSubscribeSocket client_socket(
        user.api_->Network().ZeroMQ().SubscribeSocket(client_cb_));
    if (!client_socket->Start(
            (wait_for_handshake_
                 ? user.api_->Endpoints().BlockchainPeer()
                 : user.api_->Endpoints().BlockchainPeerConnection())
                .data())) {
        throw std::runtime_error("Error connecting to client1 socket");
    }

    const auto status = done.wait_for(std::chrono::minutes(2));
    const auto future = (std::future_status::ready == status);

    return {user, added && start && future && listener_added};
}

auto Regtest_fixture_simple::GetBalance(const User& user) -> const Amount
{
    auto& account = GetHDAccount(user);
    auto& id = account.Parent().AccountID();
    const auto& widget = user.api_->UI().AccountActivity(user.nym_id_, id);
    return widget.Balance();
}

auto Regtest_fixture_simple::GetDisplayBalance(const User& user)
    -> const ot::UnallocatedCString
{
    auto& account = GetHDAccount(user);
    auto& id = account.Parent().AccountID();
    const auto& widget = user.api_->UI().AccountActivity(user.nym_id_, id);
    return widget.DisplayBalance();
}

auto Regtest_fixture_simple::GetSyncProgress(const User& user)
    -> const std::pair<int, int>
{
    auto& account = GetHDAccount(user);
    auto& id = account.Parent().AccountID();
    const auto& widget = user.api_->UI().AccountActivity(user.nym_id_, id);
    return widget.SyncProgress();
}

auto Regtest_fixture_simple::GetSyncPercentage(const User& user) -> double
{
    auto& account = GetHDAccount(user);
    auto& id = account.Parent().AccountID();
    const auto& widget = user.api_->UI().AccountActivity(user.nym_id_, id);
    return widget.SyncPercentage();
}

auto Regtest_fixture_simple::GetHDAccount(const User& user) const noexcept
    -> const bca::HD&
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
    auto& account = GetHDAccount(user);
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
        std::cout << "Waiting for synchro, balance: " << GetDisplayBalance(user)
                  << ", sync percentage: " << GetSyncPercentage(user)
                  << "%, sync progress [" << progress.first << ","
                  << progress.second << "]"
                  << ", target height: " << target << std::endl;
        if ((progress.first == target && progress.second == target) ||
            (balance == expected_balance)) {
            std::cout << "Client synchronized in "
                      << std::chrono::duration_cast<std::chrono::seconds>(
                             now - begin)
                             .count()
                      << " seconds" << std::endl;
            break;
        }
        ot::Sleep(std::chrono::seconds(5));
    }
}

}  // namespace ottest
