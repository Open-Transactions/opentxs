// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/identity/Nym.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <memory>
#include <utility>

#include "2_Factory.hpp"
#include "internal/identity/Nym.hpp"

namespace ottest
{
const bool Test_Nym::have_hd_{ot::api::crypto::HaveHDKeys()};
const bool Test_Nym::have_rsa_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Legacy)};
const bool Test_Nym::have_secp256k1_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};
const bool Test_Nym::have_ed25519_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::ED25519)};
}  // namespace ottest

namespace ottest
{
Test_Nym::Test_Nym()
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

auto Test_Nym::test_nym(
    const ot::crypto::ParameterType type,
    const ot::identity::CredentialType cred,
    const ot::identity::SourceType source,
    const ot::UnallocatedCString& name) -> bool
{
    const auto params = ot::crypto::Parameters{type, cred, source};
    const auto pNym = client_.Wallet().Nym(params, reason_, name);

    if (false == bool(pNym)) { return false; }

    const auto& nym = *pNym;

    {
        EXPECT_EQ(name, nym.Alias());
        EXPECT_TRUE(
            nym.HasCapability(ot::identity::NymCapability::SIGN_MESSAGE));
        EXPECT_TRUE(
            nym.HasCapability(ot::identity::NymCapability::ENCRYPT_MESSAGE));
        EXPECT_TRUE(nym.HasCapability(
            ot::identity::NymCapability::AUTHENTICATE_CONNECTION));
        EXPECT_TRUE(
            nym.HasCapability(ot::identity::NymCapability::SIGN_CHILDCRED));
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

auto Test_Nym::test_storage(const ot::api::session::Client& api) -> bool
{
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto alias = ot::UnallocatedCString{"alias"};
    std::unique_ptr<ot::identity::internal::Nym> pNym(ot::Factory::Nym(
        api, {}, ot::identity::Type::individual, alias, reason));

    EXPECT_TRUE(pNym);

    if (!pNym) { return false; }

    auto& nym = *pNym;
    nym.SetAlias(alias);
    const auto id = ot::identifier::Nym{nym.ID()};

    EXPECT_TRUE(nym.VerifyPseudonym());

    {
        auto bytes = ot::Space{};
        EXPECT_TRUE(nym.SerializeCredentialIndex(
            ot::writer(bytes), ot::identity::internal::Nym::Mode::Abbreviated));

        EXPECT_TRUE(api.Storage().Store(ot::reader(bytes), nym.Alias()));
    }

    {
        const auto nymList = api.Storage().NymList();

        EXPECT_EQ(1, nymList.size());

        if (1 > nymList.size()) { return false; }

        const auto& item = *nymList.begin();

        EXPECT_EQ(item.first, id.asBase58(client_.Crypto()));
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
}  // namespace ottest
