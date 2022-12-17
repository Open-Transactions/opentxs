// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class value;
}  // namespace json
}  // namespace boost
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
enum class BlockchainProfile : std::uint8_t;  // IWYU pragma: export
enum class ConnectionMode : std::int8_t;      // IWYU pragma: export

/** A list of object IDs and their associated aliases
 *  * string: id of the stored object
 *  * string: alias of the stored object
 */
using ObjectList =
    UnallocatedList<std::pair<UnallocatedCString, UnallocatedCString>>;
using SimpleCallback = std::function<void()>;
using ReadView = std::string_view;           // TODO std::span<const std::byte>
using Space = UnallocatedVector<std::byte>;  // TODO ByteArray

auto OPENTXS_EXPORT print(BlockchainProfile) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(
    const boost::json::value& jv,
    std::ostream& os,
    std::string* indent = nullptr) noexcept -> bool;
}  // namespace opentxs
