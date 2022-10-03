// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "util/ScopeGuard.hpp"  // IWYU pragma: associated

namespace opentxs
{
ScopeGuard::ScopeGuard(SimpleCallback cb) noexcept
    : post_(cb)
{
}

ScopeGuard::~ScopeGuard()
{
    try {
        if (post_) { std::invoke(post_); }
    } catch (...) {
        OT_FAIL;
    }
}
}  // namespace opentxs
