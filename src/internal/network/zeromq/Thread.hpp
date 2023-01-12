// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <thread>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::network::zeromq::internal
{
class Thread
{
public:
    virtual auto Alloc() const noexcept -> alloc::Resource* = 0;
    virtual auto ID() const noexcept -> std::thread::id = 0;

    virtual ~Thread() = default;
};
}  // namespace opentxs::network::zeromq::internal
