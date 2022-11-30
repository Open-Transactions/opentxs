// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if __has_include(<oneapi/tbb.h>)
#include <oneapi/tbb.h>  // IWYU pragma: export
#elif __has_include(<tbb/tbb.h>)
#include <tbb/tbb.h>  // IWYU pragma: export
#else
#error can not find tbb headers
#endif

namespace opentxs::tbb
{
#if __has_include(<oneapi/tbb.h>)
using namespace ::oneapi::tbb;
#else
using namespace ::tbb;
#endif
}  // namespace opentxs::tbb
