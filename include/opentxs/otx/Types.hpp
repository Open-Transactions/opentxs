// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs
{
namespace otx
{
enum class ConsensusType : std::uint8_t;      // IWYU pragma: export
enum class LastReplyStatus : std::uint8_t;    // IWYU pragma: export
enum class OTXPushType : std::uint8_t;        // IWYU pragma: export
enum class OperationType : std::uint16_t;     // IWYU pragma: export
enum class ServerReplyType : std::uint8_t;    // IWYU pragma: export
enum class ServerRequestType : std::uint8_t;  // IWYU pragma: export
}  // namespace otx

constexpr auto value(const otx::ConsensusType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::LastReplyStatus in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::OTXPushType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::OperationType in) noexcept
{
    return static_cast<std::uint16_t>(in);
}
constexpr auto value(const otx::ServerReplyType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::ServerRequestType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
}  // namespace opentxs
