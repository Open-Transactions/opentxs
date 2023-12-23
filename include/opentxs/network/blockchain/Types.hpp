// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identifier/Generic.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
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
enum class Protocol : std::uint8_t;   // IWYU pragma: export
enum class Subchain : std::uint8_t;   // IWYU pragma: export
enum class Transport : std::uint8_t;  // IWYU pragma: export

using AddressID = identifier::Generic;

OPENTXS_EXPORT auto expected_address_size(Transport) noexcept
    -> std::optional<std::size_t>;
OPENTXS_EXPORT auto print(Protocol) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Subchain) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Transport) noexcept -> std::string_view;
}  // namespace opentxs::network::blockchain
