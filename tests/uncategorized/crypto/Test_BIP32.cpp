// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/data/crypto/Bip32.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_BIP32 : public ::testing::Test
{
protected:
    using Path = ot::UnallocatedVector<ot::Bip32Index>;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;

    auto make_path(const Child::Path& path) const noexcept -> Path
    {
        auto output = Path{};
        static constexpr auto hard =
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED);

        for (const auto& item : path) {
            if (item.hardened_) {
                output.emplace_back(item.index_ | hard);
            } else {
                output.emplace_back(item.index_);
            }
        }

        return output;
    }

    Test_BIP32()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
    {
    }
};

TEST_F(Test_BIP32, init) {}

TEST_F(Test_BIP32, cases)
{
    for (const auto& item : Bip32TestCases()) {
        const auto seedID = [&] {
            const auto bytes = api_.Factory().DataFromHex(item.seed_);
            const auto seed = api_.Factory().SecretFromBytes(bytes.Bytes());

            return api_.Crypto().Seed().ImportRaw(seed, reason_);
        }();

        ASSERT_FALSE(seedID.empty());

        auto id{seedID};

        for (const auto& child : item.children_) {
            const auto key = api_.Crypto().Seed().GetHDKey(
                id,
                ot::crypto::EcdsaCurve::secp256k1,
                make_path(child.path_),
                reason_);
            auto xpub = api_.Factory().Secret(0);
            auto xprv = api_.Factory().Secret(0);

            ASSERT_TRUE(key.IsValid());
            EXPECT_EQ(seedID, id);
            EXPECT_TRUE(key.Xpub(reason_, xpub.WriteInto()));
            EXPECT_TRUE(key.Xprv(reason_, xprv.WriteInto()));
            EXPECT_EQ(child.xpub_, xpub.Bytes());
            EXPECT_EQ(child.xprv_, xprv.Bytes());
        }
    }
}

TEST_F(Test_BIP32, stress)
{
    const auto& item = Bip32TestCases().at(0u);
    const auto& child = item.children_.at(3u);
    const auto seedID = [&] {
        const auto bytes = api_.Factory().DataFromHex(item.seed_);
        const auto seed = api_.Factory().SecretFromBytes(bytes.Bytes());

        return api_.Crypto().Seed().ImportRaw(seed, reason_);
    }();

    ASSERT_FALSE(seedID.empty());

    for (auto i{0}; i < 1000; ++i) {
        auto id{seedID};
        const auto key = api_.Crypto().Seed().GetHDKey(
            id,
            ot::crypto::EcdsaCurve::secp256k1,
            make_path(child.path_),
            reason_);
        auto xpub = api_.Factory().Secret(0);
        auto xprv = api_.Factory().Secret(0);

        ASSERT_TRUE(key.IsValid());
        EXPECT_EQ(seedID, id);
        EXPECT_TRUE(key.Xpub(reason_, xpub.WriteInto()));
        EXPECT_TRUE(key.Xprv(reason_, xprv.WriteInto()));
        EXPECT_EQ(child.xpub_, xpub.Bytes());
        EXPECT_EQ(child.xprv_, xprv.Bytes());
    }
}
}  // namespace ottest
