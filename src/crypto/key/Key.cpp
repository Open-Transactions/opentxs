// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Algorithm
// IWYU pragma: no_forward_declare opentxs::crypto::symmetric::Algorithm

#include "internal/crypto/key/Key.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <robin_hood.h>

#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Mode.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Source.hpp"     // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Types.hpp"
#include "util/Container.hpp"

namespace opentxs::crypto::key
{
using AsymmetricAlgorithmMap = robin_hood::
    unordered_flat_map<asymmetric::Algorithm, proto::AsymmetricKeyType>;
using AsymmetricAlgorithmReverseMap = robin_hood::
    unordered_flat_map<proto::AsymmetricKeyType, asymmetric::Algorithm>;
using ModeMap =
    robin_hood::unordered_flat_map<asymmetric::Mode, proto::KeyMode>;
using ModeReverseMap =
    robin_hood::unordered_flat_map<proto::KeyMode, asymmetric::Mode>;
using RoleMap =
    robin_hood::unordered_flat_map<asymmetric::Role, proto::KeyRole>;
using RoleReverseMap =
    robin_hood::unordered_flat_map<proto::KeyRole, asymmetric::Role>;
using SourceMap =
    robin_hood::unordered_flat_map<symmetric::Source, proto::SymmetricKeyType>;
using SourceReverseMap =
    robin_hood::unordered_flat_map<proto::SymmetricKeyType, symmetric::Source>;
using SymmetricAlgorithmMap =
    robin_hood::unordered_flat_map<symmetric::Algorithm, proto::SymmetricMode>;
using SymmetricAlgorithmReverseMap =
    robin_hood::unordered_flat_map<proto::SymmetricMode, symmetric::Algorithm>;

auto asymmetricalgorithm_map() noexcept -> const AsymmetricAlgorithmMap&;
auto mode_map() noexcept -> const ModeMap&;
auto role_map() noexcept -> const RoleMap&;
auto source_map() noexcept -> const SourceMap&;
auto symmetricalgorithm_map() noexcept -> const SymmetricAlgorithmMap&;
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key
{
auto asymmetricalgorithm_map() noexcept -> const AsymmetricAlgorithmMap&
{
    static const auto map = AsymmetricAlgorithmMap{
        {asymmetric::Algorithm::Error, proto::AKEYTYPE_ERROR},
        {asymmetric::Algorithm::Null, proto::AKEYTYPE_NULL},
        {asymmetric::Algorithm::Legacy, proto::AKEYTYPE_LEGACY},
        {asymmetric::Algorithm::Secp256k1, proto::AKEYTYPE_SECP256K1},
        {asymmetric::Algorithm::ED25519, proto::AKEYTYPE_ED25519},
    };

    return map;
}

auto mode_map() noexcept -> const ModeMap&
{
    static const auto map = ModeMap{
        {asymmetric::Mode::Error, proto::KEYMODE_ERROR},
        {asymmetric::Mode::Null, proto::KEYMODE_NULL},
        {asymmetric::Mode::Private, proto::KEYMODE_PRIVATE},
        {asymmetric::Mode::Public, proto::KEYMODE_PUBLIC},
    };

    return map;
}

auto role_map() noexcept -> const RoleMap&
{
    static const auto map = RoleMap{
        {crypto::asymmetric::Role::Error, proto::KEYROLE_ERROR},
        {crypto::asymmetric::Role::Auth, proto::KEYROLE_AUTH},
        {crypto::asymmetric::Role::Encrypt, proto::KEYROLE_ENCRYPT},
        {crypto::asymmetric::Role::Sign, proto::KEYROLE_SIGN},
    };

    return map;
}

auto source_map() noexcept -> const SourceMap&
{
    static const auto map = SourceMap{
        {symmetric::Source::Error, proto::SKEYTYPE_ERROR},
        {symmetric::Source::Raw, proto::SKEYTYPE_RAW},
        {symmetric::Source::ECDH, proto::SKEYTYPE_ECDH},
        {symmetric::Source::Argon2i, proto::SKEYTYPE_ARGON2},
        {symmetric::Source::Argon2id, proto::SKEYTYPE_ARGON2ID},
    };

    return map;
}

auto symmetricalgorithm_map() noexcept -> const SymmetricAlgorithmMap&
{
    static const auto map = SymmetricAlgorithmMap{
        {symmetric::Algorithm::Error, proto::SMODE_ERROR},
        {symmetric::Algorithm::ChaCha20Poly1305, proto::SMODE_CHACHA20POLY1305},
    };

    return map;
}
}  // namespace opentxs::crypto::key

namespace opentxs
{
auto translate(const crypto::asymmetric::Algorithm in) noexcept
    -> proto::AsymmetricKeyType
{
    try {
        return crypto::key::asymmetricalgorithm_map().at(in);
    } catch (...) {
        return proto::AKEYTYPE_ERROR;
    }
}

auto translate(const crypto::asymmetric::Mode in) noexcept -> proto::KeyMode
{
    try {
        return crypto::key::mode_map().at(in);
    } catch (...) {
        return proto::KEYMODE_ERROR;
    }
}

auto translate(const crypto::asymmetric::Role in) noexcept -> proto::KeyRole
{
    try {
        return crypto::key::role_map().at(in);
    } catch (...) {
        return proto::KEYROLE_ERROR;
    }
}

auto translate(const crypto::symmetric::Source in) noexcept
    -> proto::SymmetricKeyType
{
    try {
        return crypto::key::source_map().at(in);
    } catch (...) {
        return proto::SKEYTYPE_ERROR;
    }
}

auto translate(const crypto::symmetric::Algorithm in) noexcept
    -> proto::SymmetricMode
{
    try {
        return crypto::key::symmetricalgorithm_map().at(in);
    } catch (...) {
        return proto::SMODE_ERROR;
    }
}

auto translate(const proto::AsymmetricKeyType in) noexcept
    -> crypto::asymmetric::Algorithm
{
    static const auto map = reverse_arbitrary_map<
        crypto::asymmetric::Algorithm,
        proto::AsymmetricKeyType,
        crypto::key::AsymmetricAlgorithmReverseMap>(
        crypto::key::asymmetricalgorithm_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Algorithm::Error;
    }
}

auto translate(const proto::KeyMode in) noexcept -> crypto::asymmetric::Mode
{
    static const auto map = reverse_arbitrary_map<
        crypto::asymmetric::Mode,
        proto::KeyMode,
        crypto::key::ModeReverseMap>(crypto::key::mode_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Mode::Error;
    }
}

auto translate(const proto::KeyRole in) noexcept -> crypto::asymmetric::Role
{
    static const auto map = reverse_arbitrary_map<
        crypto::asymmetric::Role,
        proto::KeyRole,
        crypto::key::RoleReverseMap>(crypto::key::role_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Role::Error;
    }
}

auto translate(const proto::SymmetricKeyType in) noexcept
    -> crypto::symmetric::Source
{
    static const auto map = reverse_arbitrary_map<
        crypto::symmetric::Source,
        proto::SymmetricKeyType,
        crypto::key::SourceReverseMap>(crypto::key::source_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::symmetric::Source::Error;
    }
}

auto translate(const proto::SymmetricMode in) noexcept
    -> crypto::symmetric::Algorithm
{
    static const auto map = reverse_arbitrary_map<
        crypto::symmetric::Algorithm,
        proto::SymmetricMode,
        crypto::key::SymmetricAlgorithmReverseMap>(
        crypto::key::symmetricalgorithm_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::symmetric::Algorithm::Error;
    }
}
}  // namespace opentxs
