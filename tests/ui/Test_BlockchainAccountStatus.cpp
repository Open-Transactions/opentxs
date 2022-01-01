// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <atomic>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "integration/Helpers.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
class BlockchainSelector : public ::testing::Test
{
public:
    using Protocol = ot::blockchain::crypto::HDProtocol;
    using Subaccount = ot::blockchain::crypto::SubaccountType;
    using Subchain = ot::blockchain::crypto::Subchain;
    using HDAccountMap =
        std::pmr::map<ot::OTNymID, std::pmr::map<Protocol, ot::OTIdentifier>>;
    using PCAccountMap = std::pmr::
        map<std::string, std::pmr::map<std::string, ot::OTIdentifier>>;

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
            local.nym_->PaymentCodePath(out->WriteInto());

            return out;
        }();
        pc_acct_[local.payment_code_].emplace(
            remote.payment_code_,
            api.Crypto().Blockchain().NewPaymentCodeSubaccount(
                local.nym_id_,
                api.Factory().PaymentCode(local.payment_code_),
                api.Factory().PaymentCode(remote.payment_code_),
                path->Bytes(),
                chain_,
                local.Reason()));
    }

    BlockchainSelector()
        : alice_([&]() -> auto& {
            if (false == alice_s_.has_value()) {
                const auto& v = GetVectors3().alice_;
                alice_s_.emplace(v.words_, "Alice");
                alice_s_->init(ot::Context().StartClientSession(0));
            }

            return alice_s_.value();
        }())
        , bob_([&]() -> auto& {
            if (false == bob_s_.has_value()) {
                const auto& v = GetVectors3().bob_;
                bob_s_.emplace(v.words_, "Bob");
                bob_s_->init(ot::Context().StartClientSession(1));
            }

            return bob_s_.value();
        }())
        , chris_([&]() -> auto& {
            if (false == chris_s_.has_value()) {
                chris_s_.emplace(pkt_words_, "Chris", pkt_passphrase_);
                chris_s_->init(
                    ot::Context().StartClientSession(1),
                    ot::contact::ClaimType::Individual,
                    0,
                    ot::crypto::SeedStyle::PKT);
            }

            return chris_s_.value();
        }())
    {
    }
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
    counter_alice_.expected_ += 14;
    counter_bob_.expected_ += 7;
    counter_chris_.expected_ += 4;
    alice_.api_->UI().BlockchainAccountStatus(
        alice_.nym_id_, chain_, make_cb(counter_alice_, u8"counter_alice"));
    bob_.api_->UI().BlockchainAccountStatus(
        bob_.nym_id_, chain_, make_cb(counter_bob_, u8"counter_bob"));
    chris_.api_->UI().BlockchainAccountStatus(
        chris_.nym_id_, chain_, make_cb(counter_chris_, u8"counter_chris"));
}

TEST_F(BlockchainSelector, alice_initial)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_->str(),
        chain_,
        {
            {std::string{"BIP-39 seed: "} + alice_.seed_id_ + " (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(alice_.nym_id_).at(Protocol::BIP_44)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
                 {"BIP-84: m / 84' / 1' / 0'",
                  hd_acct_.at(alice_.nym_id_).at(Protocol::BIP_84)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_->str(),
             Subaccount::PaymentCode,
             {
                 {bob_.payment_code_ + " (remote)",
                  pc_acct_.at(alice_.payment_code_)
                      .at(bob_.payment_code_)
                      ->str(),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
                 {chris_.payment_code_ + " (remote)",
                  pc_acct_.at(alice_.payment_code_)
                      .at(chris_.payment_code_)
                      ->str(),
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
        bob_.nym_id_->str(),
        chain_,
        {
            {std::string{"BIP-39 seed: "} + bob_.seed_id_ + " (default)",
             bob_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-32: m / 0'",
                  hd_acct_.at(bob_.nym_id_).at(Protocol::BIP_32)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
                 {"BIP-49: m / 49' / 1' / 0'",
                  hd_acct_.at(bob_.nym_id_).at(Protocol::BIP_49)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
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
        chris_.nym_id_->str(),
        chain_,
        {
            {std::string{"pktwallet seed: "} + chris_.seed_id_,
             chris_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(chris_.nym_id_).at(Protocol::BIP_44)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
        }};

    ASSERT_TRUE(wait_for_counter(counter_chris_));
    EXPECT_TRUE(check_blockchain_account_status(chris_, chain_, expected));
    EXPECT_TRUE(check_blockchain_account_status_qt(chris_, chain_, expected));
}

TEST_F(BlockchainSelector, new_accounts)
{
    counter_chris_.expected_ += 7;
    make_pc_account(chris_, alice_);
    make_pc_account(chris_, bob_);
}

TEST_F(BlockchainSelector, chris_final)
{
    const auto expected = BlockchainAccountStatusData{
        chris_.nym_id_->str(),
        chain_,
        {
            {std::string{"pktwallet seed: "} + chris_.seed_id_,
             chris_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  hd_acct_.at(chris_.nym_id_).at(Protocol::BIP_44)->str(),
                  {
                      {"external subchain: 0 of ? (? %)", Subchain::External},
                      {"internal subchain: 0 of ? (? %)", Subchain::Internal},
                  }},
             }},
            {chris_.payment_code_ + " (local)",
             chris_.nym_id_->str(),
             Subaccount::PaymentCode,
             {
                 {bob_.payment_code_ + " (remote)",
                  pc_acct_.at(chris_.payment_code_)
                      .at(bob_.payment_code_)
                      ->str(),
                  {
                      {"incoming subchain: 0 of ? (? %)", Subchain::Incoming},
                      {"outgoing subchain: 0 of ? (? %)", Subchain::Outgoing},
                  }},
                 {alice_.payment_code_ + " (remote)",
                  pc_acct_.at(chris_.payment_code_)
                      .at(alice_.payment_code_)
                      ->str(),
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
