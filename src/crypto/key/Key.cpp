// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/key/Key.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Mode.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Source.hpp"     // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Types.hpp"

namespace opentxs::crypto::key
{
using AsymmetricAlgorithmMap =
    frozen::unordered_map<asymmetric::Algorithm, proto::AsymmetricKeyType, 5>;
using AsymmetricAlgorithmReverseMap =
    frozen::unordered_map<proto::AsymmetricKeyType, asymmetric::Algorithm, 5>;
using ModeMap = frozen::unordered_map<asymmetric::Mode, proto::KeyMode, 4>;
using ModeReverseMap =
    frozen::unordered_map<proto::KeyMode, asymmetric::Mode, 4>;
using RoleMap = frozen::unordered_map<asymmetric::Role, proto::KeyRole, 4>;
using RoleReverseMap =
    frozen::unordered_map<proto::KeyRole, asymmetric::Role, 4>;
using SourceMap =
    frozen::unordered_map<symmetric::Source, proto::SymmetricKeyType, 5>;
using SourceReverseMap =
    frozen::unordered_map<proto::SymmetricKeyType, symmetric::Source, 5>;
using SymmetricAlgorithmMap =
    frozen::unordered_map<symmetric::Algorithm, proto::SymmetricMode, 2>;
using SymmetricAlgorithmReverseMap =
    frozen::unordered_map<proto::SymmetricMode, symmetric::Algorithm, 2>;

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
    using enum asymmetric::Algorithm;
    using enum proto::AsymmetricKeyType;
    static constexpr auto map = AsymmetricAlgorithmMap{
        {Error, AKEYTYPE_ERROR},
        {Null, AKEYTYPE_NULL},
        {Legacy, AKEYTYPE_LEGACY},
        {Secp256k1, AKEYTYPE_SECP256K1},
        {ED25519, AKEYTYPE_ED25519},
    };

    return map;
}

auto mode_map() noexcept -> const ModeMap&
{
    using enum asymmetric::Mode;
    using enum proto::KeyMode;
    static constexpr auto map = ModeMap{
        {Error, KEYMODE_ERROR},
        {Null, KEYMODE_NULL},
        {Private, KEYMODE_PRIVATE},
        {Public, KEYMODE_PUBLIC},
    };

    return map;
}

auto role_map() noexcept -> const RoleMap&
{
    using enum asymmetric::Role;
    using enum proto::KeyRole;
    static constexpr auto map = RoleMap{
        {Error, KEYROLE_ERROR},
        {Auth, KEYROLE_AUTH},
        {Encrypt, KEYROLE_ENCRYPT},
        {Sign, KEYROLE_SIGN},
    };

    return map;
}

auto source_map() noexcept -> const SourceMap&
{
    using enum symmetric::Source;
    using enum proto::SymmetricKeyType;
    static constexpr auto map = SourceMap{
        {Error, SKEYTYPE_ERROR},
        {Raw, SKEYTYPE_RAW},
        {ECDH, SKEYTYPE_ECDH},
        {Argon2i, SKEYTYPE_ARGON2},
        {Argon2id, SKEYTYPE_ARGON2ID},
    };

    return map;
}

auto symmetricalgorithm_map() noexcept -> const SymmetricAlgorithmMap&
{
    using enum symmetric::Algorithm;
    using enum proto::SymmetricMode;
    static constexpr auto map = SymmetricAlgorithmMap{
        {Error, SMODE_ERROR},
        {ChaCha20Poly1305, SMODE_CHACHA20POLY1305},
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
    static const auto map =
        frozen::invert_unordered_map(crypto::key::asymmetricalgorithm_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Algorithm::Error;
    }
}

auto translate(const proto::KeyMode in) noexcept -> crypto::asymmetric::Mode
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::mode_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Mode::Error;
    }
}

auto translate(const proto::KeyRole in) noexcept -> crypto::asymmetric::Role
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::role_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Role::Error;
    }
}

auto translate(const proto::SymmetricKeyType in) noexcept
    -> crypto::symmetric::Source
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::source_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::symmetric::Source::Error;
    }
}

auto translate(const proto::SymmetricMode in) noexcept
    -> crypto::symmetric::Algorithm
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::symmetricalgorithm_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::symmetric::Algorithm::Error;
    }
}
}  // namespace opentxs
