// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocator.hpp"

namespace opentxs
{
class Allocated
{
public:
    using allocator_type = alloc::Default;

    /// The resource returned by this function should be suitable for class
    /// users to call if they wish to allocate objects which will be passed in
    /// as member function arguments.
    virtual auto get_allocator() const noexcept -> allocator_type = 0;

    virtual ~Allocated() = default;
};
}  // namespace opentxs
