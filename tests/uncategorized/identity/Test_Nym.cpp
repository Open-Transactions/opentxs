// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/identity/Nym.hpp"

namespace ottest
{
TEST_F(Test_Nym, init_ot) {}

TEST_F(Test_Nym, storage_memdb) { EXPECT_TRUE(test_storage(client_)); }

#if OT_STORAGE_FS
TEST_F(Test_Nym, storage_fs) { EXPECT_TRUE(test_storage(client_fs_)); }
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
TEST_F(Test_Nym, storage_sqlite) { EXPECT_TRUE(test_storage(client_sqlite_)); }
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
TEST_F(Test_Nym, storage_lmdb) { EXPECT_TRUE(test_storage(client_lmdb_)); }
#endif  // OT_STORAGE_LMDB

TEST_F(Test_Nym, default_params)
{
    const auto pNym = client_.Wallet().Nym(reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::identity::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(
        nym.HasCapability(ot::identity::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(
        ot::identity::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::identity::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
    EXPECT_TRUE(nym.Name().empty());

    const auto pSection =
        claims.Section(ot::identity::wot::claim::SectionType::Scope);

    EXPECT_FALSE(pSection);
}

TEST_F(Test_Nym, secp256k1_hd_bip47)
{
    if (have_secp256k1_ && have_hd_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::secp256k1,
            ot::identity::CredentialType::HD,
            ot::identity::SourceType::Bip47));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, secp256k1_hd_self_signed)
{
    if (have_secp256k1_ && have_hd_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::secp256k1,
            ot::identity::CredentialType::HD,
            ot::identity::SourceType::PubKey));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, secp256k1_legacy_bip47)
{
    if (have_secp256k1_ && have_hd_) {
        EXPECT_FALSE(test_nym(
            ot::crypto::ParameterType::secp256k1,
            ot::identity::CredentialType::Legacy,
            ot::identity::SourceType::Bip47));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, secp256k1_legacy_self_signed)
{
    if (have_secp256k1_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::secp256k1,
            ot::identity::CredentialType::Legacy,
            ot::identity::SourceType::PubKey));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, ed25519_hd_bip47)
{
    if (have_ed25519_ && have_hd_ && have_secp256k1_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::ed25519,
            ot::identity::CredentialType::HD,
            ot::identity::SourceType::Bip47));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, ed25519_hd_self_signed)
{
    if (have_ed25519_ && have_hd_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::ed25519,
            ot::identity::CredentialType::HD,
            ot::identity::SourceType::PubKey));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, ed25519_legacy_bip47)
{
    if (have_ed25519_ && have_hd_) {
        EXPECT_FALSE(test_nym(
            ot::crypto::ParameterType::ed25519,
            ot::identity::CredentialType::Legacy,
            ot::identity::SourceType::Bip47));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, ed25519_legacy_self_signed)
{
    if (have_ed25519_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::ed25519,
            ot::identity::CredentialType::Legacy,
            ot::identity::SourceType::PubKey));
    } else {
        // TODO
    }
}

TEST_F(Test_Nym, rsa_legacy_self_signed)
{
    if (have_rsa_) {
        EXPECT_TRUE(test_nym(
            ot::crypto::ParameterType::rsa,
            ot::identity::CredentialType::Legacy,
            ot::identity::SourceType::PubKey));
    } else {
        // TODO
    }
}
}  // namespace ottest
