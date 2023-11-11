// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <source_location>
#include <sstream>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::log
{
class Streambuf final : public std::stringbuf
{
public:
    auto init(const std::source_location& loc) noexcept -> void;
    auto sync() noexcept -> int final;

    Streambuf(Log& log) noexcept;
    Streambuf() = delete;
    Streambuf(const Streambuf&) = delete;
    Streambuf(Streambuf&&) = delete;
    auto operator=(const Streambuf&) -> Streambuf& = delete;
    auto operator=(Streambuf&&) -> Streambuf& = delete;

    ~Streambuf() final = default;

private:
    Log& log_;
};
}  // namespace opentxs::log
