// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include "opentxs/util/Container.hpp"

namespace opentxs::crypto
{
// IWYU pragma: begin_exports
enum class HashType : std::uint8_t;        // IWYU pragma: keep
enum class Language : std::uint8_t;        // IWYU pragma: keep
enum class ParameterType : std::uint8_t;   // IWYU pragma: keep
enum class SecretStyle : std::uint8_t;     // IWYU pragma: keep
enum class SeedStrength : std::size_t;     // IWYU pragma: keep
enum class SeedStyle : std::uint8_t;       // IWYU pragma: keep
enum class SignatureRole : std::uint16_t;  // IWYU pragma: keep
// IWYU pragma: end_exports

enum class EcdsaCurve : std::uint8_t {
    invalid = 0,
    secp256k1 = 1,
    ed25519 = 2,
};  // IWYU pragma: export

auto print(SeedStyle) noexcept -> std::string_view;
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

// IWYU pragma: begin_exports
enum class Bip32Child : Bip32Index;    // IWYU pragma: keep
enum class Bip43Purpose : Bip32Index;  // IWYU pragma: keep
enum class Bip44Type : Bip32Index;     // IWYU pragma: keep
// IWYU pragma: end_exports
}  // namespace opentxs
