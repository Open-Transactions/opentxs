// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/PaymentCode.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <functional>
#include <future>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOState.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

bool Regtest_payment_code::init_payment_code_{false};
Server Regtest_payment_code::server_1_{};
const User Regtest_payment_code::alex_{
    GetPaymentCodeVector3().alice_.words_,
    "Alex"};
const User Regtest_payment_code::bob_{
    GetPaymentCodeVector3().bob_.words_,
    "Bob"};
TXOs Regtest_payment_code::txos_alex_{alex_};
TXOs Regtest_payment_code::txos_bob_{bob_};
std::unique_ptr<ScanListener> Regtest_payment_code::listener_alex_p_{};
std::unique_ptr<ScanListener> Regtest_payment_code::listener_bob_p_{};
}  // namespace ottest

namespace ottest
{
Regtest_payment_code::Regtest_payment_code()
    : Regtest_fixture_normal(ot_, 2)
    , api_server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
    , expected_notary_(client_1_.UI().BlockchainNotaryID(test_chain_))
    , expected_unit_(client_1_.UI().BlockchainUnitID(test_chain_))
    , expected_display_unit_(u8"UNITTEST"sv)
    , expected_account_name_(u8"On chain UNITTEST (this device)"sv)
    , expected_notary_name_(u8"Unit Test Simulation"sv)
    , memo_outgoing_("memo for outgoing transaction")
    , expected_account_type_(ot::AccountType::Blockchain)
    , expected_unit_type_(ot::UnitType::Regtest)
    , mine_to_alex_([&](Height height) -> Transaction {
        using OutputBuilder = ot::blockchain::OutputBuilder;
        static const auto baseAmmount = ot::Amount{10000000000};
        auto meta = ot::UnallocatedVector<OutpointMetadata>{};
        const auto& account = SendHD();
        auto builder = [&] {
            auto output = ot::UnallocatedVector<OutputBuilder>{};
            const auto reason = client_1_.Factory().PasswordPrompt(__func__);
            const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};
            const auto index = account.Reserve(Subchain::External, reason);

            EXPECT_TRUE(index.has_value());

            const auto& element =
                account.BalanceElement(Subchain::External, index.value_or(0));
            const auto& key = element.Key();

            opentxs::assert_true(key.IsValid());

            const auto& [bytes, value, pattern] = meta.emplace_back(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                baseAmmount,
                Pattern::PayToPubkey);
            output.emplace_back(
                value,
                miner_.Factory().BitcoinScriptP2PK(test_chain_, key, {}),
                keys);

            return output;
        }();
        auto output = miner_.Factory().BlockchainTransaction(
            test_chain_, height, builder, coinbase_fun_, 2, {});
        const auto& txid = transactions_.emplace_back(output.ID());
        auto& [bytes, amount, pattern] = meta.at(0);
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 0),
            std::forward_as_tuple(
                std::move(bytes), std::move(amount), std::move(pattern)));
        txos_alex_.AddGenerated(output, 0, account, height);

        return output;
    })
    , mine_multiple_to_alex_([&](Height height) -> Transaction {
        using Index = ot::Bip32Index;
        static constexpr auto count = 100u;
        static const auto baseAmount = ot::Amount{100000000};
        auto meta = ot::UnallocatedVector<OutpointMetadata>{};
        meta.reserve(count);
        const auto& account = SendHD();
        auto builder = [&] {
            auto output = ot::Vector<ot::blockchain::OutputBuilder>{};
            const auto reason = client_1_.Factory().PasswordPrompt(__func__);
            const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};

            for (auto i = Index{0}; i < Index{count}; ++i) {
                const auto index = account.Reserve(
                    Subchain::External, client_1_.Factory().PasswordPrompt(""));
                const auto& element = account.BalanceElement(
                    Subchain::External, index.value_or(0));
                const auto& key = element.Key();

                opentxs::assert_true(key.IsValid());

                switch (i) {
                    case 0: {
                        const auto& [bytes, value, pattern] = meta.emplace_back(
                            client_1_.Factory().DataFromBytes(
                                element.Key().PublicKey()),
                            baseAmount + i,
                            Pattern::PayToPubkey);
                        output.emplace_back(
                            value,
                            miner_.Factory().BitcoinScriptP2PK(
                                test_chain_, key, {}),
                            keys);
                    } break;
                    default: {
                        const auto& [bytes, value, pattern] = meta.emplace_back(
                            element.PubkeyHash(),
                            baseAmount + i,
                            Pattern::PayToPubkeyHash);
                        output.emplace_back(
                            value,
                            miner_.Factory().BitcoinScriptP2PKH(
                                test_chain_, key, {}),
                            keys);
                    }
                }
            }

            return output;
        }();
        auto output = miner_.Factory().BlockchainTransaction(
            test_chain_, height, builder, coinbase_fun_, 2, {});
        const auto& txid = transactions_.emplace_back(output.ID());

        for (auto i = Index{0}; i < Index{count}; ++i) {
            auto& [bytes, amount, pattern] = meta.at(i);
            expected_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(txid.Bytes(), i),
                std::forward_as_tuple(
                    std::move(bytes), std::move(amount), std::move(pattern)));
            txos_alex_.AddGenerated(output, i, account, height);
        }

        return output;
    })
    , listener_alex_([&]() -> ScanListener& {
        if (!listener_alex_p_) {
            listener_alex_p_ = std::make_unique<ScanListener>(client_1_);
        }

        opentxs::assert_false(nullptr == listener_alex_p_);

        return *listener_alex_p_;
    }())
    , listener_bob_([&]() -> ScanListener& {
        if (!listener_bob_p_) {
            listener_bob_p_ = std::make_unique<ScanListener>(client_2_);
        }

        opentxs::assert_false(nullptr == listener_bob_p_);

        return *listener_bob_p_;
    }())
{
    if (false == init_payment_code_) {
        server_1_.init(api_server_1_);
        set_introduction_server(miner_, server_1_);
        auto cb = [](User& user) {
            const auto& api = *user.api_;
            const auto& nymID = user.nym_id_;
            const auto reason = api.Factory().PasswordPrompt(__func__);
            api.Crypto().Blockchain().NewHDSubaccount(
                nymID,
                ot::blockchain::crypto::HDProtocol::BIP_44,
                test_chain_,
                reason);
        };
        auto& alex = const_cast<User&>(alex_);
        auto& bob = const_cast<User&>(bob_);
        alex.init_custom(client_1_, server_1_, cb);
        bob.init_custom(client_2_, server_1_, cb);

        opentxs::assert_true(
            alex_.payment_code_ ==
            GetPaymentCodeVector3().alice_.payment_code_);
        opentxs::assert_true(
            bob_.payment_code_ == GetPaymentCodeVector3().bob_.payment_code_);

        init_payment_code_ = true;
    }
}

auto Regtest_payment_code::CheckContactID(
    const User& local,
    const User& remote,
    const ot::UnallocatedCString& paymentcode) const noexcept -> bool
{
    const auto& api = *local.api_;
    const auto n2c = api.Contacts().ContactID(remote.nym_id_);
    const auto p2c = api.Contacts().PaymentCodeToContact(
        api.Factory().PaymentCodeFromBase58(paymentcode), test_chain_);

    EXPECT_EQ(n2c, p2c);
    EXPECT_EQ(
        p2c.asBase58(ot_.Crypto()),
        local.Contact(remote.name_).asBase58(ot_.Crypto()));

    auto output{true};
    output &= (n2c == p2c);
    output &=
        (p2c.asBase58(ot_.Crypto()) ==
         local.Contact(remote.name_).asBase58(ot_.Crypto()));

    return output;
}

auto Regtest_payment_code::CheckOTXResult(
    opentxs::api::session::OTX::BackgroundTask result) const noexcept -> bool
{
    auto& [id, future] = result;
    auto out = (id > 0);
    const auto [status, message] = future.get();
    using enum opentxs::otx::LastReplyStatus;
    out &= (MessageSuccess == status);
    out &= message.operator bool();

    EXPECT_GT(id, 0);
    EXPECT_EQ(status, MessageSuccess);
    EXPECT_TRUE(message);

    return out;
}

auto Regtest_payment_code::CheckTXODBAlex() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_alex_.Extract(out);

        return out;
    }();

    return TestWallet(client_1_, state);
}

auto Regtest_payment_code::CheckTXODBBob() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_bob_.Extract(out);

        return out;
    }();

    return TestWallet(client_2_, state);
}

auto Regtest_payment_code::ExtractElements(
    const ot::blockchain::protocol::bitcoin::base::block::Block& block) noexcept
    -> ot::blockchain::block::Elements
{
    return block.Internal().ExtractElements(FilterType::ES, {});
}

auto Regtest_payment_code::ReceiveHD() const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(bob_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_payment_code::ReceivePC() const noexcept
    -> const ot::blockchain::crypto::PaymentCode&
{
    return client_2_.Crypto()
        .Blockchain()
        .Account(bob_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_payment_code::SendHD() const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alex_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_payment_code::SendPC() const noexcept
    -> const ot::blockchain::crypto::PaymentCode&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alex_.nym_id_, test_chain_)
        .GetPaymentCode()
        .at(0);
}

auto Regtest_payment_code::Shutdown() noexcept -> void
{
    listener_bob_p_.reset();
    listener_alex_p_.reset();
    transactions_.clear();
    Regtest_fixture_normal::Shutdown();
}
}  // namespace ottest
