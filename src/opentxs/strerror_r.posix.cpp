// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/strerror_r.hpp"  // IWYU pragma: associated

#if __has_include(<features.h>)
extern "C" {
#include <features.h>
}
#endif

#include <cstring>
#include <span>

#include "internal/util/P0330.hpp"

namespace opentxs
{
using namespace std::literals;

static auto error_code_to_string(std::span<char> buf, int ec) noexcept
    -> std::size_t
{
#ifdef __USE_GNU

    return std::strlen(::strerror_r(ec, buf.data(), buf.size()));
#else
    const auto size = ::strerror_r(ec, buf.data(), buf.size());

    if (0 <= size) {

        return static_cast<std::size_t>(size);
    } else {

        return 0_uz;
    }
#endif
}

template <typename T, std::size_t buf_size = 1024_uz>
auto get_error_string(T& out, int ec) noexcept -> void
{
    out.resize(buf_size);
    out.resize(error_code_to_string(out, ec));

    if (out.empty()) {
        out = "unknown error code: "s.append(std::to_string(ec));
    }
}

auto error_code_to_string(int ec) noexcept -> UnallocatedCString
{
    auto out = UnallocatedCString{};
    get_error_string(out, ec);

    return out;
}

auto error_code_to_string(int ec, alloc::Strategy alloc) noexcept -> CString
{
    auto out = CString{alloc.result_};
    get_error_string(out, ec);

    return out;
}
}  // namespace opentxs
