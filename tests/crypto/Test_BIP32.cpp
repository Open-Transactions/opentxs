// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "crypto/Bip32Vectors.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_BIP32 : public ::testing::Test
{
protected:
    using Path = ot::UnallocatedVector<ot::Bip32Index>;

    const ot::api::session::Client& api_;
    const ot::OTPasswordPrompt reason_;

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
    for (const auto& item : bip32_test_cases_) {
        const auto seedID = [&] {
            const auto bytes =
                api_.Factory().Data(item.seed_, ot::StringStyle::Hex);
            const auto seed = api_.Factory().SecretFromBytes(bytes->Bytes());

            return api_.Crypto().Seed().ImportRaw(seed, reason_);
        }();

        ASSERT_FALSE(seedID.empty());

        auto id{seedID};

        for (const auto& child : item.children_) {
            const auto pKey = api_.Crypto().Seed().GetHDKey(
                id, ot::EcdsaCurve::secp256k1, make_path(child.path_), reason_);
            const auto& key = *pKey;

            ASSERT_TRUE(pKey);
            EXPECT_EQ(seedID, id);
            EXPECT_EQ(child.xpub_, key.Xpub(reason_));
            EXPECT_EQ(child.xprv_, key.Xprv(reason_));
        }
    }
}

TEST_F(Test_BIP32, stress)
{
    const auto& item = bip32_test_cases_.at(0u);
    const auto& child = item.children_.at(3u);
    const auto seedID = [&] {
        const auto bytes =
            api_.Factory().Data(item.seed_, ot::StringStyle::Hex);
        const auto seed = api_.Factory().SecretFromBytes(bytes->Bytes());

        return api_.Crypto().Seed().ImportRaw(seed, reason_);
    }();

    ASSERT_FALSE(seedID.empty());

    for (auto i{0}; i < 1000; ++i) {
        auto id{seedID};
        const auto pKey = api_.Crypto().Seed().GetHDKey(
            id, ot::EcdsaCurve::secp256k1, make_path(child.path_), reason_);
        const auto& key = *pKey;

        ASSERT_TRUE(pKey);
        EXPECT_EQ(seedID, id);
        EXPECT_EQ(child.xpub_, key.Xpub(reason_));
        EXPECT_EQ(child.xprv_, key.Xprv(reason_));
    }
}
}  // namespace ottest
