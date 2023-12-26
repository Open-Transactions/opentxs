// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::contract::peer
{
enum class ConnectionInfoType : std::uint32_t;  // IWYU pragma: export
enum class ObjectType : std::uint32_t;          // IWYU pragma: export
enum class RequestType : std::uint32_t;         // IWYU pragma: export
enum class SecretType : std::uint32_t;          // IWYU pragma: export

OPENTXS_EXPORT auto print(ConnectionInfoType) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(ObjectType) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(RequestType) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(SecretType) noexcept -> std::string_view;
}  // namespace opentxs::contract::peer
