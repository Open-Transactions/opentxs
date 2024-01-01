// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/key/Key.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/Enums.pb.h>
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
using AsymmetricAlgorithmMap = frozen::
    unordered_map<asymmetric::Algorithm, protobuf::AsymmetricKeyType, 5>;
using AsymmetricAlgorithmReverseMap = frozen::
    unordered_map<protobuf::AsymmetricKeyType, asymmetric::Algorithm, 5>;
using ModeMap = frozen::unordered_map<asymmetric::Mode, protobuf::KeyMode, 4>;
using ModeReverseMap =
    frozen::unordered_map<protobuf::KeyMode, asymmetric::Mode, 4>;
using RoleMap = frozen::unordered_map<asymmetric::Role, protobuf::KeyRole, 4>;
using RoleReverseMap =
    frozen::unordered_map<protobuf::KeyRole, asymmetric::Role, 4>;
using SourceMap =
    frozen::unordered_map<symmetric::Source, protobuf::SymmetricKeyType, 5>;
using SourceReverseMap =
    frozen::unordered_map<protobuf::SymmetricKeyType, symmetric::Source, 5>;
using SymmetricAlgorithmMap =
    frozen::unordered_map<symmetric::Algorithm, protobuf::SymmetricMode, 2>;
using SymmetricAlgorithmReverseMap =
    frozen::unordered_map<protobuf::SymmetricMode, symmetric::Algorithm, 2>;

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
    using enum protobuf::AsymmetricKeyType;
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
    using enum protobuf::KeyMode;
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
    using enum protobuf::KeyRole;
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
    using enum protobuf::SymmetricKeyType;
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
    using enum protobuf::SymmetricMode;
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
    -> protobuf::AsymmetricKeyType
{
    try {
        return crypto::key::asymmetricalgorithm_map().at(in);
    } catch (...) {
        return protobuf::AKEYTYPE_ERROR;
    }
}

auto translate(const crypto::asymmetric::Mode in) noexcept -> protobuf::KeyMode
{
    try {
        return crypto::key::mode_map().at(in);
    } catch (...) {
        return protobuf::KEYMODE_ERROR;
    }
}

auto translate(const crypto::asymmetric::Role in) noexcept -> protobuf::KeyRole
{
    try {
        return crypto::key::role_map().at(in);
    } catch (...) {
        return protobuf::KEYROLE_ERROR;
    }
}

auto translate(const crypto::symmetric::Source in) noexcept
    -> protobuf::SymmetricKeyType
{
    try {
        return crypto::key::source_map().at(in);
    } catch (...) {
        return protobuf::SKEYTYPE_ERROR;
    }
}

auto translate(const crypto::symmetric::Algorithm in) noexcept
    -> protobuf::SymmetricMode
{
    try {
        return crypto::key::symmetricalgorithm_map().at(in);
    } catch (...) {
        return protobuf::SMODE_ERROR;
    }
}

auto translate(const protobuf::AsymmetricKeyType in) noexcept
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

auto translate(const protobuf::KeyMode in) noexcept -> crypto::asymmetric::Mode
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::mode_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Mode::Error;
    }
}

auto translate(const protobuf::KeyRole in) noexcept -> crypto::asymmetric::Role
{
    static const auto map =
        frozen::invert_unordered_map(crypto::key::role_map());

    try {
        return map.at(in);
    } catch (...) {
        return crypto::asymmetric::Role::Error;
    }
}

auto translate(const protobuf::SymmetricKeyType in) noexcept
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

auto translate(const protobuf::SymmetricMode in) noexcept
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
