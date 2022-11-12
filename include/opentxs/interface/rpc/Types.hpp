// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::rpc
{
using TypeEnum = std::uint32_t;

enum class AccountEventType : TypeEnum;
enum class AccountType : TypeEnum;
enum class CommandType : TypeEnum;
enum class ContactEventType : TypeEnum;
enum class PaymentType : TypeEnum;
enum class PushType : TypeEnum;
enum class ResponseCode : TypeEnum;

OPENTXS_EXPORT auto print(AccountEventType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(AccountType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(CommandType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(ContactEventType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(PaymentType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(PushType value) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(ResponseCode value) noexcept -> std::string_view;

constexpr auto value(AccountEventType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(AccountType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(CommandType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(ContactEventType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(PaymentType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(PushType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
constexpr auto value(ResponseCode type) noexcept
{
    return static_cast<TypeEnum>(type);
}
}  // namespace opentxs::rpc
