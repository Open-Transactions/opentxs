// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <optional>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/PasswordPrompt.hpp"

#define TEST_MASTER_PASSWORD "test password"
#define TEST_PLAINTEXT "The quick brown fox jumped over the lazy dog."

namespace ot = opentxs;

namespace ottest
{
bool init_{false};

struct Test_Symmetric : public ::testing::Test {
    static const ot::crypto::key::symmetric::Algorithm mode_;
    static ot::identifier::Nym alice_nym_id_;
    static ot::identifier::Nym bob_nym_id_;
    static ot::OTSymmetricKey key_;
    static ot::OTSymmetricKey second_key_;
    static std::optional<ot::Secret> key_password_;
    static ot::Space ciphertext_;
    static ot::Space second_ciphertext_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    ot::Nym_p alice_;
    ot::Nym_p bob_;

    Test_Symmetric()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
        , alice_()
        , bob_()
    {
        if (false == init_) { init(); }

        alice_ = api_.Wallet().Nym(alice_nym_id_);
        bob_ = api_.Wallet().Nym(bob_nym_id_);
    }

    void init()
    {
        const auto seedA = api_.InternalClient().Exec().Wallet_ImportSeed(
            "spike nominee miss inquiry fee nothing belt list other "
            "daughter leave valley twelve gossip paper",
            "");
        const auto seedB = api_.InternalClient().Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        alice_nym_id_ = api_.Wallet().Nym({seedA, 0}, reason_, "Alice")->ID();
        bob_nym_id_ = api_.Wallet().Nym({seedB, 0}, reason_, "Bob")->ID();
        key_password_ = api_.Factory().SecretFromText(TEST_MASTER_PASSWORD);
        init_ = true;
    }
};

const ot::crypto::key::symmetric::Algorithm Test_Symmetric::mode_{
    ot::crypto::key::symmetric::Algorithm::ChaCha20Poly1305};
ot::identifier::Nym Test_Symmetric::alice_nym_id_{};
ot::identifier::Nym Test_Symmetric::bob_nym_id_{};
ot::OTSymmetricKey Test_Symmetric::key_{ot::crypto::key::Symmetric::Factory()};
ot::OTSymmetricKey Test_Symmetric::second_key_{
    ot::crypto::key::Symmetric::Factory()};
std::optional<ot::Secret> Test_Symmetric::key_password_{};
ot::Space Test_Symmetric::ciphertext_{};
ot::Space Test_Symmetric::second_ciphertext_{};

TEST_F(Test_Symmetric, create_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password.Internal().SetPassword(key_password_.value()));

    key_ = api_.Crypto().Symmetric().Key(password, mode_);

    EXPECT_TRUE(key_.get());
}

TEST_F(Test_Symmetric, key_functionality)
{
    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password.Internal().SetPassword(key_password_.value()));

    const auto encrypted = key_->Encrypt(
        TEST_PLAINTEXT, password, ot::writer(ciphertext_), true, mode_);

    ASSERT_TRUE(encrypted);

    auto recoveredKey =
        api_.Crypto().Symmetric().Key(ot::reader(ciphertext_), mode_);

    ASSERT_TRUE(recoveredKey.get());

    ot::UnallocatedCString plaintext{};
    auto decrypted = recoveredKey->Decrypt(
        ot::reader(ciphertext_), password, [&](const auto size) {
            plaintext.resize(size);

            return ot::WritableView{plaintext.data(), plaintext.size()};
        });

    ASSERT_TRUE(decrypted);
    EXPECT_STREQ(TEST_PLAINTEXT, plaintext.c_str());

    auto wrongPassword = api_.Factory().SecretFromText("not the password");

    ASSERT_TRUE(password.Internal().SetPassword(wrongPassword));

    recoveredKey =
        api_.Crypto().Symmetric().Key(ot::reader(ciphertext_), mode_);

    ASSERT_TRUE(recoveredKey.get());

    decrypted = recoveredKey->Decrypt(
        ot::reader(ciphertext_), password, [&](const auto size) {
            plaintext.resize(size);

            return ot::WritableView{plaintext.data(), plaintext.size()};
        });

    EXPECT_FALSE(decrypted);
}

TEST_F(Test_Symmetric, create_second_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(password.Internal().SetPassword(key_password_.value()));

    second_key_ = api_.Crypto().Symmetric().Key(password, mode_);

    EXPECT_TRUE(second_key_.get());

    const auto encrypted = second_key_->Encrypt(
        TEST_PLAINTEXT, password, ot::writer(second_ciphertext_), true, mode_);

    ASSERT_TRUE(encrypted);
}
}  // namespace ottest
