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

#if defined(BOOST_ALL_NO_LIB)
// NOTE some versions of clang in some build configurations will emit a "macro
// is not used" warning regarding BOOST_ALL_NO_LIB if a target is linked against
// Boost::headers but some of the translation units in the target do not
// actually include any boost headers
#endif

#ifndef opentxs_EXPORTS
#ifndef OPENTXS_STATIC_DEFINE
#error opentxs_EXPORTS or OPENTXS_STATIC_DEFINE must be defined when building the library
#endif
#endif
