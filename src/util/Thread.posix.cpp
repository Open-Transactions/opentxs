// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Thread.hpp"  // IWYU pragma: associated

extern "C" {
#include <unistd.h>
}

namespace opentxs
{
auto PageSize() noexcept -> std::size_t { return ::getpagesize(); }
}  // namespace opentxs
