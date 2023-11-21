// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Context.hpp"  // IWYU pragma: associated

#include <utility>

#include "TBB.hpp"

namespace opentxs
{
auto RunJob(SimpleCallback cb) noexcept -> void
{
    tbb::fire_and_forget(std::move(cb));
}
}  // namespace opentxs
