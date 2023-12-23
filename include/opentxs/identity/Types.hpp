// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
enum class CredentialRole : std::uint32_t;   // IWYU pragma: export
enum class CredentialType : std::uint32_t;   // IWYU pragma: export
enum class NymCapability : std::uint8_t;     // IWYU pragma: export
enum class SourceProofType : std::uint32_t;  // IWYU pragma: export
enum class SourceType : std::uint32_t;       // IWYU pragma: export
enum class Type : std::uint32_t;             // IWYU pragma: export

OPENTXS_EXPORT auto print(CredentialRole value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(CredentialType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SourceProofType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SourceType value) noexcept -> std::string_view;

constexpr auto value(CredentialRole in) noexcept
{
    return static_cast<std::underlying_type_t<CredentialRole>>(in);
}
}  // namespace opentxs::identity

namespace opentxs
{
using Nym_p = std::shared_ptr<const identity::Nym>;
}  // namespace opentxs
