// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// NOTE without this you may get winsock-related problems with boost::asio on
// Windows:
//     https://stackoverflow.com/q/9750344
//     https://stackoverflow.com/a/38201394
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <boost/asio.hpp>           // IWYU pragma: export
#include <boost/beast/core.hpp>     // IWYU pragma: export
#include <boost/beast/http.hpp>     // IWYU pragma: export
#include <boost/beast/ssl.hpp>      // IWYU pragma: export
#include <boost/beast/version.hpp>  // IWYU pragma: export

#pragma GCC diagnostic pop
