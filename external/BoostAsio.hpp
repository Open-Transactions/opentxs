// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#ifdef _WIN32
// NOTE without this you may get winsock-related problems with boost::asio on
// Windows:
//     https://stackoverflow.com/q/9750344
//     https://stackoverflow.com/a/38201394
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// NOTE
// https://stackoverflow.com/q/70348706
// https://devblogs.microsoft.com/oldnewthing/20230111-00/?p=107694
#ifndef _ALLOW_COROUTINE_ABI_MISMATCH
#define _ALLOW_COROUTINE_ABI_MISMATCH
#endif
#endif

#include <boost/asio.hpp>           // IWYU pragma: export
#include <boost/beast/core.hpp>     // IWYU pragma: export
#include <boost/beast/http.hpp>     // IWYU pragma: export
#include <boost/beast/ssl.hpp>      // IWYU pragma: export
#include <boost/beast/version.hpp>  // IWYU pragma: export

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto unexpected_asio_error(const boost::system::error_code& ec) noexcept
    -> bool;
}  // namespace opentxs
