// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <utility>

#include "2_Factory.hpp"
#include "internal/identity/Identity.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/ParameterType.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace
{
class Test_Nym : public ::testing::Test
{
public:
    static const bool have_hd_;
    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;

    const ot::api::session::Client& client_;
#if OT_STORAGE_FS
    const ot::api::session::Client& client_fs_;
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
    const ot::api::session::Client& client_sqlite_;
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
    const ot::api::session::Client& client_lmdb_;
#endif  // OT_STORAGE_LMDB
    const ot::OTPasswordPrompt reason_;

    bool test_nym(
        const ot::crypto::ParameterType type,
        const ot::identity::CredentialType cred,
        const ot::identity::SourceType source,
        const ot::UnallocatedCString& name = "Nym")
    {
        const auto params = ot::crypto::Parameters{type, cred, source};
        const auto pNym = client_.Wallet().Nym(params, reason_, name);

        if (false == bool(pNym)) { return false; }

        const auto& nym = *pNym;

        {
            EXPECT_EQ(name, nym.Alias());
            EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
            EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
            EXPECT_TRUE(
                nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
            EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
            EXPECT_EQ(1, nym.Revision());
            EXPECT_EQ(name, nym.Name());
            EXPECT_EQ(source, nym.Source().Type());
        }

        {
            const auto& claims = nym.Claims();
            const auto pSection =
                claims.Section(ot::identity::wot::claim::SectionType::Scope);

            EXPECT_TRUE(pSection);

            if (false == bool(pSection)) { return false; }

            const auto& section = *pSection;

            EXPECT_EQ(1, section.Size());

            const auto pGroup =
                section.Group(ot::identity::wot::claim::ClaimType::Individual);

            EXPECT_TRUE(pGroup);

            if (false == bool(pGroup)) { return false; }

            const auto& group = *pGroup;
            const auto pItem = group.PrimaryClaim();

            EXPECT_TRUE(pItem);

            if (false == bool(pItem)) { return false; }

            const auto& item = *pItem;

            EXPECT_EQ(name, item.Value());
        }

        return true;
    }

    bool test_storage(const ot::api::session::Client& api)
    {
        const auto reason = api.Factory().PasswordPrompt(__func__);
        const auto alias = ot::UnallocatedCString{"alias"};
        std::unique_ptr<ot::identity::internal::Nym> pNym(ot::Factory::Nym(
            api, {}, ot::identity::Type::individual, alias, reason));

        EXPECT_TRUE(pNym);

        if (!pNym) { return false; }

        auto& nym = *pNym;
        nym.SetAlias(alias);
        const auto id = ot::OTNymID{nym.ID()};

        EXPECT_TRUE(nym.VerifyPseudonym());

        {
            auto bytes = ot::Space{};
            EXPECT_TRUE(nym.SerializeCredentialIndex(
                ot::writer(bytes),
                ot::identity::internal::Nym::Mode::Abbreviated));

            EXPECT_TRUE(api.Storage().Store(ot::reader(bytes), nym.Alias()));
        }

        {
            const auto nymList = api.Storage().NymList();

            EXPECT_EQ(1, nymList.size());

            if (1 > nymList.size()) { return false; }

            const auto& item = *nymList.begin();

            EXPECT_EQ(item.first, id->str());
            EXPECT_EQ(item.second, alias);
        }

        {
            auto bytes = ot::Space{};

            EXPECT_TRUE(api.Storage().LoadNym(id, ot::writer(bytes)));
            EXPECT_TRUE(ot::valid(ot::reader(bytes)));

            pNym.reset(ot::Factory::Nym(api, ot::reader(bytes), alias));

            EXPECT_TRUE(pNym);

            if (!pNym) { return false; }

            const auto& loadedNym = *pNym;

            EXPECT_TRUE(loadedNym.CompareID(id));
            EXPECT_TRUE(loadedNym.VerifyPseudonym());
        }

        return true;
    }

    Test_Nym()
        : client_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(0)))
#if OT_STORAGE_FS
        , client_fs_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(
                  ot::Options{}.SetStoragePlugin("fs"),
                  1)))
#endif  // OT_STORAGE_FS
#if OT_STORAGE_SQLITE
        , client_sqlite_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(
                  ot::Options{}.SetStoragePlugin("sqlite"),
                  2)))
#endif  // OT_STORAGE_SQLITE
#if OT_STORAGE_LMDB
        , client_lmdb_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(
                  ot::Options{}.SetStoragePlugin("lmdb"),
                  3)))
#endif  // OT_STORAGE_LMDB
        , reason_(client_.Factory().PasswordPrompt(__func__))
    {
    }
};

const bool Test_Nym::have_hd_{ot::api::crypto::HaveHDKeys()};
const bool Test_Nym::have_rsa_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::Legacy)};
const bool Test_Nym::have_secp256k1_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::Secp256k1)};
const bool Test_Nym::have_ed25519_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::ED25519)};

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
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
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
}  // namespace
