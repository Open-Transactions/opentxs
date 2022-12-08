// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/regtest/HD.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOState.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;

const User Regtest_fixture_hd::alice_{
    GetPaymentCodeVector3().alice_.words_,
    "Alice"};
TXOs Regtest_fixture_hd::txos_{alice_};
std::unique_ptr<ScanListener> Regtest_fixture_hd::listener_p_{};
}  // namespace ottest

namespace ottest
{
Regtest_fixture_hd::Regtest_fixture_hd()
    : Regtest_fixture_normal(ot_, 1)
    , expected_notary_(client_1_.UI().BlockchainNotaryID(test_chain_))
    , expected_unit_(client_1_.UI().BlockchainUnitID(test_chain_))
    , expected_display_unit_(u8"UNITTEST"_sv)
    , expected_account_name_(u8"On chain UNITTEST (this device)"_sv)
    , expected_notary_name_(u8"Unit Test Simulation"_sv)
    , memo_outgoing_("memo for outgoing transaction")
    , expected_account_type_(ot::AccountType::Blockchain)
    , expected_unit_type_(ot::UnitType::Regtest)
    , hd_generator_([&](Height height) -> Transaction {
        using OutputBuilder = ot::blockchain::OutputBuilder;
        using Index = ot::Bip32Index;
        static constexpr auto count = 100u;
        static const auto baseAmount = ot::blockchain::Amount{100000000};
        auto meta = ot::UnallocatedVector<OutpointMetadata>{};
        meta.reserve(count);
        const auto& account = SendHD();
        auto output = miner_.Factory().BitcoinGenerationTransaction(
            test_chain_,
            height,
            [&] {
                auto out = ot::UnallocatedVector<OutputBuilder>{};
                const auto reason =
                    client_1_.Factory().PasswordPrompt(__func__);
                const auto keys =
                    ot::UnallocatedSet<ot::blockchain::crypto::Key>{};

                for (auto i = Index{0}; i < Index{count}; ++i) {
                    const auto index = account.Reserve(
                        Subchain::External,
                        client_1_.Factory().PasswordPrompt(""));
                    const auto& element = account.BalanceElement(
                        Subchain::External, index.value_or(0));
                    const auto& key = element.Key();

                    OT_ASSERT(key.IsValid());

                    switch (i) {
                        case 0: {
                            const auto& [bytes, value, pattern] =
                                meta.emplace_back(
                                    client_1_.Factory().DataFromBytes(
                                        element.Key().PublicKey()),
                                    baseAmount + i,
                                    Pattern::PayToPubkey);
                            out.emplace_back(
                                value,
                                miner_.Factory().BitcoinScriptP2PK(
                                    test_chain_, key),
                                keys);
                        } break;
                        default: {
                            const auto& [bytes, value, pattern] =
                                meta.emplace_back(
                                    element.PubkeyHash(),
                                    baseAmount + i,
                                    Pattern::PayToPubkeyHash);
                            out.emplace_back(
                                value,
                                miner_.Factory().BitcoinScriptP2PKH(
                                    test_chain_, key),
                                keys);
                        }
                    }
                }

                return out;
            }(),
            coinbase_fun_);

        OT_ASSERT(output);

        const auto& txid = transactions_.emplace_back(output->ID());

        for (auto i = Index{0}; i < Index{count}; ++i) {
            auto& [bytes, amount, pattern] = meta.at(i);
            expected_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(txid.Bytes(), i),
                std::forward_as_tuple(
                    std::move(bytes), std::move(amount), std::move(pattern)));
            txos_.AddGenerated(*output, i, account, height);
        }

        return output;
    })
    , listener_([&]() -> ScanListener& {
        if (!listener_p_) {
            listener_p_ = std::make_unique<ScanListener>(client_1_);
        }

        OT_ASSERT(listener_p_);

        return *listener_p_;
    }())
{
    if (false == init_) {
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
        auto& alice = const_cast<User&>(alice_);
        alice.init_custom(client_1_, cb);

        OT_ASSERT(
            alice_.payment_code_ ==
            GetPaymentCodeVector3().alice_.payment_code_);

        init_ = true;
    }
}

auto Regtest_fixture_hd::CheckTXODB() const noexcept -> bool
{
    const auto state = [&] {
        auto out = TXOState{};
        txos_.Extract(out);

        return out;
    }();

    return TestWallet(client_1_, state);
}

auto Regtest_fixture_hd::SendHD() const noexcept
    -> const ot::blockchain::crypto::HD&
{
    return client_1_.Crypto()
        .Blockchain()
        .Account(alice_.nym_id_, test_chain_)
        .GetHD()
        .at(0);
}

auto Regtest_fixture_hd::Shutdown() noexcept -> void
{
    listener_p_.reset();
    transactions_.clear();
    Regtest_fixture_normal::Shutdown();
}
}  // namespace ottest
