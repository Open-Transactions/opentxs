// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
// IWYU pragma: begin_exports
enum class Protocol : std::uint8_t;   // IWYU pragma: keep
enum class Subchain : std::uint8_t;   // IWYU pragma: keep
enum class Transport : std::uint8_t;  // IWYU pragma: keep
// IWYU pragma: end_exports

using AddressID = identifier::Generic;

OPENTXS_EXPORT auto print(Protocol) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Subchain) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Transport) noexcept -> std::string_view;
}  // namespace opentxs::network::blockchain
