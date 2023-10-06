// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/SendPayment_blockchain.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/regtest/Normal.hpp"

namespace ottest
{
namespace ot = opentxs;

ot::Nym_p RPC_BC::alex_p_{};
ot::UnallocatedDeque<ot::blockchain::block::TransactionHash>
    RPC_BC::transactions_{};
std::unique_ptr<ScanListener> RPC_BC::listener_p_{};

auto RPC_BC::Cleanup() noexcept -> void
{
    listener_p_.reset();
    transactions_.clear();
    alex_p_.reset();
    Regtest_fixture_normal::Shutdown();
    RPC_fixture::Cleanup();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"  // NOLINT
RPC_BC::RPC_BC()
    : Regtest_fixture_normal(ot_, 1)
    , alex_([&]() -> const ot::identity::Nym& {
        if (!alex_p_) {
            const auto reason = client_1_.Factory().PasswordPrompt(__func__);

            alex_p_ = client_1_.Wallet().Nym(reason, "Alex");

            opentxs::assert_false(nullptr == alex_p_);

            client_1_.Crypto().Blockchain().NewHDSubaccount(
                alex_p_->ID(),
                ot::blockchain::crypto::HDProtocol::BIP_44,
                test_chain_,
                reason);
        }

        opentxs::assert_false(nullptr == alex_p_);

        return *alex_p_;
    }())
    , account_(client_1_.Crypto()
                   .Blockchain()
                   .Account(alex_.ID(), test_chain_)
                   .GetHD()
                   .at(0))
    , mine_to_alex_([&](Height height) -> Transaction {
        using OutputBuilder = ot::blockchain::OutputBuilder;
        static const auto baseAmount = ot::Amount{10000000000};
        auto builder = [&] {
            auto output = ot::UnallocatedVector<OutputBuilder>{};
            const auto reason = client_1_.Factory().PasswordPrompt(__func__);
            const auto keys = ot::UnallocatedSet<ot::blockchain::crypto::Key>{};
            const auto index = account_.Reserve(Subchain::External, reason);

            EXPECT_TRUE(index.has_value());

            const auto& element =
                account_.BalanceElement(Subchain::External, index.value_or(0));
            const auto& key = element.Key();

            opentxs::assert_true(key.IsValid());

            output.emplace_back(
                baseAmount,
                miner_.Factory().BitcoinScriptP2PK(test_chain_, key, {}),
                keys);

            return output;
        }();
        auto output = miner_.Factory().BlockchainTransaction(
            test_chain_, height, builder, coinbase_fun_, 2, {});
        transactions_.emplace_back(output.ID());

        return output;
    })
    , listener_([&]() -> ScanListener& {
        if (!listener_p_) {
            listener_p_ = std::make_unique<ScanListener>(client_1_);
        }

        opentxs::assert_false(nullptr == listener_p_);

        return *listener_p_;
    }())
{
}
#pragma GCC diagnostic pop

}  // namespace ottest
