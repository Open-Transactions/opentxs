// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <type_traits>

namespace opentxs::otx
{
enum class ConsensusType : std::uint8_t;      // IWYU pragma: export
enum class LastReplyStatus : std::uint8_t;    // IWYU pragma: export
enum class OperationType : std::uint16_t;     // IWYU pragma: export
enum class PushType : std::uint8_t;           // IWYU pragma: export
enum class ServerReplyType : std::uint8_t;    // IWYU pragma: export
enum class ServerRequestType : std::uint8_t;  // IWYU pragma: export

constexpr auto value(ConsensusType in) noexcept
{
    return static_cast<std::underlying_type_t<ConsensusType>>(in);
}
constexpr auto value(LastReplyStatus in) noexcept
{
    return static_cast<std::underlying_type_t<LastReplyStatus>>(in);
}
constexpr auto value(OperationType in) noexcept
{
    return static_cast<std::underlying_type_t<OperationType>>(in);
}
constexpr auto value(PushType in) noexcept
{
    return static_cast<std::underlying_type_t<PushType>>(in);
}
constexpr auto value(ServerReplyType in) noexcept
{
    return static_cast<std::underlying_type_t<ServerReplyType>>(in);
}
constexpr auto value(ServerRequestType in) noexcept
{
    return static_cast<std::underlying_type_t<ServerRequestType>>(in);
}
}  // namespace opentxs::otx
