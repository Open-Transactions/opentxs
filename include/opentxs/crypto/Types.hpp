// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <functional>

#include "opentxs/util/Container.hpp"

namespace opentxs::crypto
{
enum class HashType : std::uint8_t;        // IWYU pragma: export
enum class Language : std::uint8_t;        // IWYU pragma: export
enum class ParameterType : std::uint8_t;   // IWYU pragma: export
enum class SecretStyle : std::uint8_t;     // IWYU pragma: export
enum class SeedStrength : std::size_t;     // IWYU pragma: export
enum class SeedStyle : std::uint8_t;       // IWYU pragma: export
enum class SignatureRole : std::uint16_t;  // IWYU pragma: export
enum class EcdsaCurve : std::uint8_t {
    invalid = 0,
    secp256k1 = 1,
    ed25519 = 2,
};
}  // namespace opentxs::crypto

namespace opentxs
{
using GetPreimage = std::function<UnallocatedCString()>;
using Bip32Network = std::uint32_t;
using Bip32Depth = std::uint8_t;
using Bip32Fingerprint = std::uint32_t;
using Bip32Index = std::uint32_t;

using BIP44Chain = bool;
static const BIP44Chain INTERNAL_CHAIN = true;
static const BIP44Chain EXTERNAL_CHAIN = false;

enum class Bip32Child : Bip32Index;    // IWYU pragma: export
enum class Bip43Purpose : Bip32Index;  // IWYU pragma: export
enum class Bip44Type : Bip32Index;     // IWYU pragma: export

auto print(crypto::SeedStyle) noexcept -> UnallocatedCString;
}  // namespace opentxs
