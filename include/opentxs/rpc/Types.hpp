// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"

namespace opentxs::rpc
{
using SessionIndex = int;

enum class AccountEventType : std::uint32_t;  // IWYU pragma: export
enum class AccountType : std::uint32_t;       // IWYU pragma: export
enum class CommandType : std::uint32_t;       // IWYU pragma: export
enum class ContactEventType : std::uint32_t;  // IWYU pragma: export
enum class PaymentType : std::uint32_t;       // IWYU pragma: export
enum class PushType : std::uint32_t;          // IWYU pragma: export
enum class ResponseCode : std::uint32_t;      // IWYU pragma: export

OPENTXS_EXPORT auto print(AccountEventType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(AccountType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(CommandType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(ContactEventType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(PaymentType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(PushType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(ResponseCode value) noexcept -> std::string_view;

constexpr auto value(AccountEventType type) noexcept
{
    return static_cast<std::underlying_type_t<AccountEventType>>(type);
}
constexpr auto value(AccountType type) noexcept
{
    return static_cast<std::underlying_type_t<AccountType>>(type);
}
constexpr auto value(CommandType type) noexcept
{
    return static_cast<std::underlying_type_t<CommandType>>(type);
}
constexpr auto value(ContactEventType type) noexcept
{
    return static_cast<std::underlying_type_t<ContactEventType>>(type);
}
constexpr auto value(PaymentType type) noexcept
{
    return static_cast<std::underlying_type_t<PaymentType>>(type);
}
constexpr auto value(PushType type) noexcept
{
    return static_cast<std::underlying_type_t<PushType>>(type);
}
constexpr auto value(ResponseCode type) noexcept
{
    return static_cast<std::underlying_type_t<ResponseCode>>(type);
}
}  // namespace opentxs::rpc
