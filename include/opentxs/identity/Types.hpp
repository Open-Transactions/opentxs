// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace opentxs::identity
{
enum class CredentialRole : std::uint32_t;
enum class CredentialType : std::uint32_t;
enum class Type : std::uint32_t;
enum class SourceProofType : std::uint32_t;
enum class SourceType : std::uint32_t;
}  // namespace opentxs::identity

namespace opentxs
{
OPENTXS_EXPORT auto print(identity::CredentialRole value) noexcept
    -> std::string_view;
OPENTXS_EXPORT auto print(identity::CredentialType value) noexcept
    -> std::string_view;
OPENTXS_EXPORT auto print(identity::Type value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(identity::SourceProofType value) noexcept
    -> std::string_view;
OPENTXS_EXPORT auto print(identity::SourceType value) noexcept
    -> std::string_view;
}  // namespace opentxs
