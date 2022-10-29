// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <span>

#include "ottest/fixtures/crypto/Symmetric.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(Test_Symmetric, create_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(SetPassword(password));

    key_ = api_.Crypto().Symmetric().Key(mode_, password);

    EXPECT_TRUE(key_);
}

TEST_F(Test_Symmetric, key_functionality)
{
    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(SetPassword(password));

    const auto encrypted = key_.Encrypt(
        plaintext_, ot::writer(ciphertext_), mode_, password, true);

    ASSERT_TRUE(encrypted);

    auto recoveredKey =
        api_.Crypto().Symmetric().Key(ot::reader(ciphertext_), mode_);

    ASSERT_TRUE(recoveredKey);

    ot::UnallocatedCString plaintext{};
    auto decrypted = recoveredKey.Decrypt(
        ot::reader(ciphertext_),
        {[&](const auto size) -> ot::WriteBuffer {
            plaintext.resize(size);
            auto* out = reinterpret_cast<std::byte*>(plaintext.data());

            return std::span<std::byte>{out, plaintext.size()};
        }},
        password);

    ASSERT_TRUE(decrypted);
    EXPECT_EQ(plaintext_, plaintext);

    auto wrongPassword = api_.Factory().SecretFromText("not the password");

    ASSERT_TRUE(SetPassword(wrongPassword, password));

    recoveredKey =
        api_.Crypto().Symmetric().Key(ot::reader(ciphertext_), mode_);

    ASSERT_TRUE(recoveredKey);

    decrypted = recoveredKey.Decrypt(
        ot::reader(ciphertext_),
        {[&](const auto size) -> ot::WriteBuffer {
            plaintext.resize(size);
            auto* out = reinterpret_cast<std::byte*>(plaintext.data());

            return std::span<std::byte>{out, plaintext.size()};
        }},
        password);

    EXPECT_FALSE(decrypted);
}

TEST_F(Test_Symmetric, create_second_key)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    auto password = api_.Factory().PasswordPrompt("");

    ASSERT_TRUE(SetPassword(password));

    second_key_ = api_.Crypto().Symmetric().Key(mode_, password);

    EXPECT_TRUE(second_key_);

    const auto encrypted = second_key_.Encrypt(
        plaintext_, ot::writer(second_ciphertext_), mode_, password, true);

    ASSERT_TRUE(encrypted);
}
}  // namespace ottest
