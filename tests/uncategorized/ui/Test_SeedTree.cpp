// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <optional>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/SeedTree.hpp"

namespace ottest
{
class Test_SeedTree : public ::testing::Test
{
public:
    static constexpr auto seed_1_id_{
        "ot2xku1FsJryQkxUnHpY7thRc7ActebtanLZcfTs3rGaUfyTefpsTne"};
    static constexpr auto seed_2_id_{
        "ot2xkue7cSsxNN4wGoz3wvoTNFyFTbMfLfXd8Bk6yqQE4awnhWsezNM"};
    static constexpr auto seed_3_id_{
        "ot2xkuFMFyL3JbDg8377CWX7eNNga7X7KHi11qF7xKZNTAYeXXwmgi3"};
    static constexpr auto alex_name_{"Alex"};
    static constexpr auto bob_name_{"Bob"};
    static constexpr auto chris_name_{"Chris"};
    static constexpr auto daniel_name_{"Daniel"};
    static constexpr auto pkt_words_{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto pkt_passphrase_{"Password123#"};

    static Counter counter_;
    static std::optional<User> alex_;
    static std::optional<User> bob_;
    static std::optional<User> chris_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;

    Test_SeedTree()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
    {
    }
};

Counter Test_SeedTree::counter_{};
std::optional<User> Test_SeedTree::alex_{std::nullopt};
std::optional<User> Test_SeedTree::bob_{std::nullopt};
std::optional<User> Test_SeedTree::chris_{std::nullopt};

TEST_F(Test_SeedTree, initialize_opentxs) { init_seed_tree(api_, counter_); }

TEST_F(Test_SeedTree, empty)
{
    const auto expected = SeedTreeData{};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, create_nyms)
{
    counter_.expected_ += 7;

    {
        const auto& v = GetPaymentCodeVector3().alice_;
        alex_.emplace(v.words_, alex_name_);
        alex_->init(api_);
    }
    {
        const auto& v = GetPaymentCodeVector3().alice_;
        bob_.emplace(v.words_, bob_name_);
        bob_->init(api_, ot::identity::Type::individual, 1);
    }
    {
        chris_.emplace(pkt_words_, chris_name_, pkt_passphrase_);
        chris_->init(
            api_,
            ot::identity::Type::individual,
            0,
            ot::crypto::SeedStyle::PKT);
    }

    const auto expected = SeedTreeData{{
        {seed_1_id_,
         "Unnamed seed: BIP-39 (default)",
         ot::crypto::SeedStyle::BIP39,
         {
             {0,
              alex_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{alex_name_} + " (default)"},
             {1, bob_->nym_id_.asBase58(alex_->api_->Crypto()), bob_name_},
         }},
        {seed_2_id_,
         "Unnamed seed: Legacy pktwallet",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, add_seed)
{
    counter_.expected_ += 1;
    const auto& v = GetPaymentCodeVector3().bob_;
    api_.Crypto().Seed().ImportSeed(
        api_.Factory().SecretFromText(v.words_),
        api_.Factory().Secret(0),
        ot::crypto::SeedStyle::BIP39,
        ot::crypto::Language::en,
        reason_,
        "Imported");

    const auto expected = SeedTreeData{{
        {seed_1_id_,
         "Unnamed seed: BIP-39 (default)",
         ot::crypto::SeedStyle::BIP39,
         {
             {0,
              alex_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{alex_name_} + " (default)"},
             {1, bob_->nym_id_.asBase58(alex_->api_->Crypto()), bob_name_},
         }},
        {seed_3_id_, "Imported: BIP-39", ot::crypto::SeedStyle::BIP39, {}},
        {seed_2_id_,
         "Unnamed seed: Legacy pktwallet",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, rename_nym)
{
    counter_.expected_ += 1;
    {
        auto editor = api_.Wallet().mutable_Nym(alex_->nym_id_, reason_);
        editor.SetScope(editor.Type(), daniel_name_, true, reason_);
    }

    const auto expected = SeedTreeData{{
        {seed_1_id_,
         "Unnamed seed: BIP-39 (default)",
         ot::crypto::SeedStyle::BIP39,
         {
             {0,
              alex_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{daniel_name_} + " (default)"},
             {1, bob_->nym_id_.asBase58(alex_->api_->Crypto()), bob_name_},
         }},
        {seed_3_id_, "Imported: BIP-39", ot::crypto::SeedStyle::BIP39, {}},
        {seed_2_id_,
         "Unnamed seed: Legacy pktwallet",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, rename_seed)
{
    counter_.expected_ += 1;
    api_.Crypto().Seed().SetSeedComment(
        api_.Factory().IdentifierFromBase58(seed_2_id_), "Backup");
    const auto expected = SeedTreeData{{
        {seed_1_id_,
         "Unnamed seed: BIP-39 (default)",
         ot::crypto::SeedStyle::BIP39,
         {
             {0,
              alex_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{daniel_name_} + " (default)"},
             {1, bob_->nym_id_.asBase58(alex_->api_->Crypto()), bob_name_},
         }},
        {seed_2_id_,
         "Backup: Legacy pktwallet",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
        {seed_3_id_, "Imported: BIP-39", ot::crypto::SeedStyle::BIP39, {}},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, change_default_seed)
{
    counter_.expected_ += 3;
    api_.Crypto().Seed().SetDefault(
        api_.Factory().IdentifierFromBase58(seed_2_id_));
    const auto expected = SeedTreeData{{
        {seed_2_id_,
         "Backup: Legacy pktwallet (default)",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
        {seed_3_id_, "Imported: BIP-39", ot::crypto::SeedStyle::BIP39, {}},
        {seed_1_id_,
         "Unnamed seed: BIP-39",
         ot::crypto::SeedStyle::BIP39,
         {
             {0,
              alex_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{daniel_name_} + " (default)"},
             {1, bob_->nym_id_.asBase58(alex_->api_->Crypto()), bob_name_},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}

TEST_F(Test_SeedTree, change_default_nym)
{
    counter_.expected_ += 3;
    api_.Wallet().SetDefaultNym(bob_->nym_id_);
    const auto expected = SeedTreeData{{
        {seed_2_id_,
         "Backup: Legacy pktwallet (default)",
         ot::crypto::SeedStyle::PKT,
         {
             {0, chris_->nym_id_.asBase58(alex_->api_->Crypto()), chris_name_},
         }},
        {seed_3_id_, "Imported: BIP-39", ot::crypto::SeedStyle::BIP39, {}},
        {seed_1_id_,
         "Unnamed seed: BIP-39",
         ot::crypto::SeedStyle::BIP39,
         {
             {0, alex_->nym_id_.asBase58(alex_->api_->Crypto()), daniel_name_},
             {1,
              bob_->nym_id_.asBase58(alex_->api_->Crypto()),
              ot::UnallocatedCString{bob_name_} + " (default)"},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_seed_tree(api_, expected));
    EXPECT_TRUE(check_seed_tree_qt(api_, expected));
}
}  // namespace ottest
