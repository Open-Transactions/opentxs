// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identifier/Generic.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <string_view>
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

namespace opentxs
{
namespace identifier
{
class Generic;  // IWYU pragma: keep
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using ContactID = identifier::Generic;
using OTZMQWorkType = std::uint16_t;
/** A list of object IDs and their associated aliases
 *  * string: id of the stored object
 *  * string: alias of the stored object
 */
using ObjectList =
    UnallocatedList<std::pair<UnallocatedCString, UnallocatedCString>>;
using ReadView = std::string_view;  // TODO std::span<const std::byte>
using SimpleCallback = std::function<void()>;
using Space = UnallocatedVector<std::byte>;  // TODO ByteArray

struct OPENTXS_EXPORT HexType {
};

static constexpr auto IsHex = HexType{};

enum class AccountType : std::int8_t;         // IWYU pragma: export
enum class AddressType : std::uint8_t;        // IWYU pragma: export
enum class BlockchainProfile : std::uint8_t;  // IWYU pragma: export
enum class ConnectionMode : std::int8_t;      // IWYU pragma: export

inline namespace unittype
{
enum class UnitType : std::uint32_t;  // IWYU pragma: export
}  // namespace unittype

enum class WorkType : OTZMQWorkType;  // IWYU pragma: export

auto OPENTXS_EXPORT print(AccountType) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(AddressType) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(BlockchainProfile) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(OTZMQWorkType in) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(UnitType) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(WorkType in) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(
    const boost::json::value& jv,
    std::ostream& os,
    std::string* indent = nullptr) noexcept -> bool;

constexpr auto value(ConnectionMode val) noexcept -> std::int8_t
{
    return static_cast<std::int8_t>(val);
}
constexpr auto value(const WorkType in) noexcept
{
    return static_cast<OTZMQWorkType>(in);
}
}  // namespace opentxs
