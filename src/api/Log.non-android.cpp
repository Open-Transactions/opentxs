// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Log.hpp"  // IWYU pragma: associated

#include <iostream>

#include "internal/util/Log.hpp"

namespace opentxs::api::imp
{
auto Log::print(
    const int,
    const Console console,
    const std::string_view text,
    const std::string_view thread) noexcept -> void
{
    auto& out = [&]() -> auto& {
        if (Console::err == console) {

            return std::cerr;
        } else {

            return std::cout;
        }
    }();
    out << "(" << thread << ") ";
    out << text << '\n';
    out.flush();
}
}  // namespace opentxs::api::imp
