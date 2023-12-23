// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identifier/HDSeed.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class HDSeed;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
using Bip32Network = std::uint32_t;
using Bip32Depth = std::uint8_t;
using Bip32Fingerprint = std::uint32_t;
using Bip32Index = std::uint32_t;
using SeedID = identifier::HDSeed;

enum class Bip32Child : Bip32Index;        // IWYU pragma: export
enum class Bip43Purpose : Bip32Index;      // IWYU pragma: export
enum class EcdsaCurve : std::uint8_t;      // IWYU pragma: export
enum class HashType : std::uint8_t;        // IWYU pragma: export
enum class Language : std::uint8_t;        // IWYU pragma: export
enum class ParameterType : std::uint8_t;   // IWYU pragma: export
enum class SecretStyle : std::uint8_t;     // IWYU pragma: export
enum class SeedStrength : std::size_t;     // IWYU pragma: export
enum class SeedStyle : std::uint8_t;       // IWYU pragma: export
enum class SignatureRole : std::uint16_t;  // IWYU pragma: export

OPENTXS_EXPORT auto print(Language) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SeedStyle) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SeedStrength) noexcept -> std::string_view;

constexpr auto value(const HashType in) noexcept
{
    return static_cast<std::underlying_type_t<HashType>>(in);
}
}  // namespace opentxs::crypto
