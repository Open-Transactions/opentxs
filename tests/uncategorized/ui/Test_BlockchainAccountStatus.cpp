// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"

namespace ottest
{
class BlockchainSelector : public ::testing::Test
{
public:
    using Protocol = ot::blockchain::crypto::HDProtocol;
    using Subaccount = ot::blockchain::crypto::SubaccountType;
    using Subchain = ot::blockchain::crypto::Subchain;
    using HDAccountMap = ot::UnallocatedMap<
        ot::identifier::Nym,
        ot::UnallocatedMap<Protocol, ot::identifier::Generic>>;
    using PCAccountMap = std::map<
        ot::UnallocatedCString,
        ot::UnallocatedMap<ot::UnallocatedCString, ot::identifier::Generic>>;

    static constexpr auto chain_{ot::blockchain::Type::UnitTest};
    static constexpr auto pkt_words_{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto pkt_passphrase_{"Password123#"};

    static std::optional<User> alice_s_;
    static std::optional<User> bob_s_;
    static std::optional<User> chris_s_;
    static HDAccountMap hd_acct_;
    static PCAccountMap pc_acct_;

    const User& alice_;
    const User& bob_;
    const User& chris_;

    auto Account(const User& user, ot::blockchain::Type chain) const noexcept
        -> const ot::blockchain::crypto::Account&
    {
        return user.api_->Crypto().Blockchain().Account(user.nym_id_, chain);
    }

    auto make_hd_account(const User& user, const Protocol type) noexcept -> void
    {
        hd_acct_[user.nym_id_].emplace(
            type,
            user.api_->Crypto().Blockchain().NewHDSubaccount(
                user.nym_id_, type, chain_, user.Reason()));
    }
    auto make_pc_account(const User& local, const User& remote) noexcept -> void
    {
        const auto& api = *local.api_;
        const auto path = [&] {
            auto out = api.Factory().Data();
            local.nym_->PaymentCodePath(out.WriteInto());

            return out;
        }();
        pc_acct_[local.payment_code_].emplace(
            remote.payment_code_,
            api.Crypto()
                .Blockchain()
                .LoadOrCreateSubaccount(
                    local.nym_id_,
                    api.Factory().PaymentCodeFromBase58(remote.payment_code_),
                    chain_,
                    local.Reason())
                .ID());
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"  // NOLINT
    BlockchainSelector()
        : alice_([&]() -> auto& {
            if (false == alice_s_.has_value()) {
                const auto& v = GetPaymentCodeVector3().alice_;
                alice_s_.emplace(v.words_, "Alice");
                alice_s_->init(
                    OTTestEnvironment::GetOT().StartClientSession(0));
            }

            return alice_s_.value();
        }())
        , bob_([&]() -> auto& {
            if (false == bob_s_.has_value()) {
                const auto& v = GetPaymentCodeVector3().bob_;
                bob_s_.emplace(v.words_, "Bob");
                bob_s_->init(OTTestEnvironment::GetOT().StartClientSession(1));
            }

            return bob_s_.value();
        }())
        , chris_([&]() -> auto& {
            if (false == chris_s_.has_value()) {
                chris_s_.emplace(pkt_words_, "Chris", pkt_passphrase_);
                chris_s_->init(
                    OTTestEnvironment::GetOT().StartClientSession(1),
                    ot::identity::Type::individual,
                    0,
                    ot::crypto::SeedStyle::PKT);
            }

            return chris_s_.value();
        }())
    {
    }
#pragma GCC diagnostic pop
};

std::optional<User> BlockchainSelector::alice_s_{std::nullopt};
std::optional<User> BlockchainSelector::bob_s_{std::nullopt};
std::optional<User> BlockchainSelector::chris_s_{std::nullopt};
BlockchainSelector::HDAccountMap BlockchainSelector::hd_acct_{};
BlockchainSelector::PCAccountMap BlockchainSelector::pc_acct_{};
Counter counter_alice_{};
Counter counter_bob_{};
Counter counter_chris_{};

TEST_F(BlockchainSelector, initial_conditions)
{
    make_hd_account(alice_, Protocol::BIP_44);
    make_hd_account(alice_, Protocol::BIP_84);
    make_hd_account(bob_, Protocol::BIP_32);
    make_hd_account(bob_, Protocol::BIP_49);
    make_hd_account(chris_, Protocol::BIP_44);
    make_pc_account(alice_, bob_);
    make_pc_account(alice_, chris_);
    counter_alice_.expected_ += 16;
    counter_bob_.expected_ += 10;
    counter_chris_.expected_ += 7;
    init_blockchain_account_status(alice_, chain_, counter_alice_);
    init_blockchain_account_status(bob_, chain_, counter_bob_);
    init_blockchain_account_status(chris_, chain_, counter_chris_);
}

TEST_F(BlockchainSelector, alice_initial)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(alice_.api_->Crypto()),
        chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(alice_.nym_id_)
                      .at(Protocol::BIP_44)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
                 {"BIP-84: m / 84' / 1' / 0'",
                  hd_acct_.at(alice_.nym_id_)
                      .at(Protocol::BIP_84)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_,
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"version 3 subchain: 0 of ? (? %)",
                       Subchain::NotificationV3},
                  }},
                 {bob_.payment_code_ + " (remote)",
                  pc_acct_.at(alice_.payment_code_)
                      .at(bob_.payment_code_)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
                 {chris_.payment_code_ + " (remote)",
                  pc_acct_.at(alice_.payment_code_)
                      .at(chris_.payment_code_)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
             }},
        }};

    ASSERT_TRUE(wait_for_counter(counter_alice_));
    EXPECT_TRUE(check_blockchain_account_status(alice_, chain_, expected));
    EXPECT_TRUE(check_blockchain_account_status_qt(alice_, chain_, expected));
}

TEST_F(BlockchainSelector, bob_initial)
{
    const auto expected = BlockchainAccountStatusData{
        bob_.nym_id_.asBase58(alice_.api_->Crypto()),
        chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             bob_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-32: m / 0'",
                  hd_acct_.at(bob_.nym_id_)
                      .at(Protocol::BIP_32)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
                 {"BIP-49: m / 49' / 1' / 0'",
                  hd_acct_.at(bob_.nym_id_)
                      .at(Protocol::BIP_49)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {bob_.payment_code_ + " (local)",
             bob_.nym_id_,
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(bob_, chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"version 3 subchain: 0 of ? (? %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};

    ASSERT_TRUE(wait_for_counter(counter_bob_));
    EXPECT_TRUE(check_blockchain_account_status(bob_, chain_, expected));
    EXPECT_TRUE(check_blockchain_account_status_qt(bob_, chain_, expected));
}

TEST_F(BlockchainSelector, chris_initial)
{
    const auto expected = BlockchainAccountStatusData{
        chris_.nym_id_.asBase58(alice_.api_->Crypto()),
        chain_,
        {
            {"Unnamed seed: Legacy pktwallet",
             chris_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(chris_.nym_id_)
                      .at(Protocol::BIP_44)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {chris_.payment_code_ + " (local)",
             chris_.nym_id_,
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(chris_, chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"version 3 subchain: 0 of ? (? %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};

    ASSERT_TRUE(wait_for_counter(counter_chris_));
    EXPECT_TRUE(check_blockchain_account_status(chris_, chain_, expected));
    EXPECT_TRUE(check_blockchain_account_status_qt(chris_, chain_, expected));
}

TEST_F(BlockchainSelector, new_accounts)
{
    counter_chris_.expected_ += 6;
    make_pc_account(chris_, alice_);
    make_pc_account(chris_, bob_);
}

TEST_F(BlockchainSelector, chris_final)
{
    const auto expected = BlockchainAccountStatusData{
        chris_.nym_id_.asBase58(alice_.api_->Crypto()),
        chain_,
        {
            {"Unnamed seed: Legacy pktwallet",
             chris_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(chris_.nym_id_)
                      .at(Protocol::BIP_44)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {chris_.payment_code_ + " (local)",
             chris_.nym_id_,
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(chris_, chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"version 3 subchain: 0 of ? (? %)",
                       Subchain::NotificationV3},
                  }},
                 {bob_.payment_code_ + " (remote)",
                  pc_acct_.at(chris_.payment_code_)
                      .at(bob_.payment_code_)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
                 {alice_.payment_code_ + " (remote)",
                  pc_acct_.at(chris_.payment_code_)
                      .at(alice_.payment_code_)
                      .asBase58(alice_.api_->Crypto()),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
             }},
        }};

    ASSERT_TRUE(wait_for_counter(counter_chris_));
    EXPECT_TRUE(check_blockchain_account_status(chris_, chain_, expected));
    EXPECT_TRUE(check_blockchain_account_status_qt(chris_, chain_, expected));
}
}  // namespace ottest
