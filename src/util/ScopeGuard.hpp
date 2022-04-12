// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
class ScopeGuard
{
public:
    ScopeGuard(SimpleCallback cb) noexcept;
    template <typename Pre, typename Post>
    ScopeGuard(Pre pre, Post post) noexcept
        : ScopeGuard(std::move(post))
    {
        try {
            std::invoke(pre);
        } catch (...) {
            OT_FAIL;
        }
    }

    ~ScopeGuard();

private:
    const SimpleCallback post_;
};
}  // namespace opentxs
