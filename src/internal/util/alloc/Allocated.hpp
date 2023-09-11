// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Container.hpp"

#pragma once

#include "opentxs/util/Allocated.hpp"

namespace opentxs::pmr
{
class Allocated : virtual public opentxs::Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final
    {
        return allocator_;
    }

    // NOTE: it's tempting to define an get_deleter here so you don't need to
    // implement it in every child class. Don't do it. get_deleter must always
    // be defined in the most derived class or else you get undefined behavior.

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
}  // namespace opentxs::pmr
