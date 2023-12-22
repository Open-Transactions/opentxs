// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::inline literals
{
#if defined(__cpp_char8_t)
inline auto operator"" _sv(const char8_t* ptr, std::size_t size)
    -> std::string_view
{
    static_assert(sizeof(char8_t) == sizeof(char));

    return {reinterpret_cast<const char*>(ptr), size};
}

inline auto operator"" _cstr(const char8_t* ptr, std::size_t) -> const char*
{
    static_assert(sizeof(char8_t) == sizeof(char));

    return reinterpret_cast<const char*>(ptr);
}
#endif
}  // namespace opentxs::inline literals
