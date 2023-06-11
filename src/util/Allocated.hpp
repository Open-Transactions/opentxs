// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocated.hpp"

#include "opentxs/util/Container.hpp"

namespace opentxs::implementation
{
class Allocated : virtual public opentxs::Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final
    {
        return allocator_;
    }

    Allocated(const Allocated&) = delete;
    Allocated(Allocated&&) = delete;

    ~Allocated() override = default;

protected:
    allocator_type allocator_;

    Allocated(allocator_type alloc) noexcept
        : allocator_(std::move(alloc))
    {
    }
};
}  // namespace opentxs::implementation
