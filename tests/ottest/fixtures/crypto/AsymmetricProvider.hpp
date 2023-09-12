// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
class AsymmetricProvider;
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals;

class OPENTXS_EXPORT Signatures : public ::testing::Test
{
public:
    using Role = ot::crypto::asymmetric::Role;
    using Type = ot::crypto::asymmetric::Algorithm;

    static const bool have_hd_;
    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;

    const ot::api::session::Client& api_;
    const ot::crypto::SeedID seed_id_;
    const ot::crypto::HashType sha256_{ot::crypto::HashType::Sha256};
    const ot::crypto::HashType sha512_{ot::crypto::HashType::Sha512};
    const ot::crypto::HashType blake160_{ot::crypto::HashType::Blake2b160};
    const ot::crypto::HashType blake256_{ot::crypto::HashType::Blake2b256};
    const ot::crypto::HashType blake512_{ot::crypto::HashType::Blake2b512};
    const ot::crypto::HashType ripemd160_{ot::crypto::HashType::Ripemd160};
    const ot::UnallocatedCString plaintext_string_1_{"Test string"};
    const ot::UnallocatedCString plaintext_string_2_{"Another string"};
    const ot::ByteArray plaintext_1_{
        plaintext_string_1_.data(),
        plaintext_string_1_.size()};
    const ot::ByteArray plaintext_2_{
        plaintext_string_2_.data(),
        plaintext_string_2_.size()};
    ot::crypto::asymmetric::Key ed_;
    ot::crypto::asymmetric::Key ed_hd_;
    ot::crypto::asymmetric::Key ed_2_;
    ot::crypto::asymmetric::Key secp_;
    ot::crypto::asymmetric::Key secp_hd_;
    ot::crypto::asymmetric::Key secp_2_;
    ot::crypto::asymmetric::Key rsa_sign_1_;
    ot::crypto::asymmetric::Key rsa_sign_2_;

    [[maybe_unused]] Signatures();

    static auto get_hd_key(
        const ot::api::session::Client& api,
        const ot::crypto::SeedID& seedID,
        const ot::crypto::EcdsaCurve& curve,
        const std::uint32_t index = 0) -> ot::crypto::asymmetric::Key;
    [[maybe_unused]] static auto get_key(
        const ot::api::session::Client& api,
        const ot::crypto::EcdsaCurve curve,
        const Role role) -> ot::crypto::asymmetric::Key;

    [[maybe_unused]] auto test_dh(
        const ot::crypto::AsymmetricProvider& lib,
        const ot::crypto::asymmetric::Key& keyOne,
        const ot::crypto::asymmetric::Key& keyTwo,
        const ot::Data& expected) -> bool;
    [[maybe_unused]] auto test_signature(
        const ot::Data& plaintext,
        const ot::crypto::AsymmetricProvider& lib,
        const ot::crypto::asymmetric::Key& key,
        const ot::crypto::HashType hash) -> bool;
    [[maybe_unused]] auto bad_signature(
        const ot::crypto::AsymmetricProvider& lib,
        const ot::crypto::asymmetric::Key& key,
        const ot::crypto::HashType hash) -> bool;
};
}  // namespace ottest
