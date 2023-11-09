// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <functional>

#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"

namespace ottest
{
using enum opentxs::blockchain::crypto::SubaccountType;

Counter counter_alice_{};
Counter counter_bob_{};
Counter counter_chris_{};

TEST_F(BlockchainAccountStatus, initial_conditions)
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

TEST_F(BlockchainAccountStatus, alice_initial)
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
                      .GetSubaccounts(Notification)
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

TEST_F(BlockchainAccountStatus, bob_initial)
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
                      .GetSubaccounts(Notification)
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

TEST_F(BlockchainAccountStatus, chris_initial)
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
                      .GetSubaccounts(Notification)
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

TEST_F(BlockchainAccountStatus, new_accounts)
{
    counter_chris_.expected_ += 6;
    make_pc_account(chris_, alice_);
    make_pc_account(chris_, bob_);
}

TEST_F(BlockchainAccountStatus, chris_final)
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
                      .GetSubaccounts(Notification)
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
