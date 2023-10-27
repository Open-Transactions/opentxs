// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <span>

#include "ottest/data/crypto/Hashes.hpp"
#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
TEST_F(OneClientSession, uncompressed_key)
{
    using enum opentxs::crypto::asymmetric::Algorithm;
    using opentxs::ByteArray;

    const auto& factory = client_1_.Factory();
    const auto reason = factory.PasswordPrompt(__func__);

    for (const auto& [sec, pub, pkh, last20, cs, addr] : EthHashes()) {
        const auto decoded = ByteArray{opentxs::IsHex, sec};
        const auto key = factory.AsymmetricKey(
            Secp256k1, factory.SecretFromBytes(decoded.Bytes()), reason, {});
        const auto& ec = key.asEllipticCurve();
        const auto& secp256k1 = ec.asSecp256k1();
        auto serialized = secp256k1.UncompressedPubkey();
        serialized.remove_prefix(1);
        const auto uncompressed = ByteArray{serialized};

        EXPECT_TRUE(key.IsValid());
        EXPECT_EQ(key.Type(), Secp256k1);
        EXPECT_TRUE(ec.IsValid());
        EXPECT_TRUE(secp256k1.IsValid());
        EXPECT_EQ(uncompressed.asHex(), pub);
    }
}
}  // namespace ottest
