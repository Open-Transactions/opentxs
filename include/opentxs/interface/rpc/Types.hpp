// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
namespace rpc
{
using TypeEnum = std::uint32_t;

enum class AccountEventType : TypeEnum;  // IWYU pragma: export
enum class AccountType : TypeEnum;       // IWYU pragma: export
enum class CommandType : TypeEnum;       // IWYU pragma: export
enum class ContactEventType : TypeEnum;  // IWYU pragma: export
enum class PaymentType : TypeEnum;       // IWYU pragma: export
enum class PushType : TypeEnum;          // IWYU pragma: export
enum class ResponseCode : TypeEnum;      // IWYU pragma: export
}  // namespace rpc

OPENTXS_EXPORT auto print(rpc::AccountEventType value) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::AccountType value) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::CommandType value) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::ContactEventType value) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::PaymentType value) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::PushType value) noexcept -> UnallocatedCString;
OPENTXS_EXPORT auto print(rpc::ResponseCode value) noexcept
    -> UnallocatedCString;

constexpr auto value(rpc::AccountEventType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::AccountType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::CommandType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::ContactEventType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::PaymentType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::PushType type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
constexpr auto value(rpc::ResponseCode type) noexcept
{
    return static_cast<rpc::TypeEnum>(type);
}
}  // namespace opentxs
