// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

namespace opentxs
{
enum class Console { err, out };            // IWYU pragma: export
enum class LogAction { flush, terminate };  // IWYU pragma: export
}  // namespace opentxs

namespace opentxs::internal
{
class Log
{
public:
    static constexpr auto flush_ = std::byte{0x00};
    static constexpr auto terminate_ = std::byte{0x01};

    static auto Endpoint() noexcept -> const char*;
    static auto SetVerbosity(const int level) noexcept -> void;
    static auto Shutdown() noexcept -> void;
    static auto Start() noexcept -> void;

    Log() = default;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    auto operator=(const Log&) -> Log& = delete;
    auto operator=(Log&&) -> Log& = delete;

    virtual ~Log() = default;
};
}  // namespace opentxs::internal
