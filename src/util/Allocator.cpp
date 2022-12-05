// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/Allocator.hpp"  // IWYU pragma: associated
#include "util/Allocator.hpp"          // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <boost/container/pmr/global_resource.hpp>
#include <cstddef>
#include <new>
#include <utility>

#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::alloc
{
auto System() noexcept -> Resource*
{
    // TODO replace with std::pmr::new_delete_resource once Android and Apple
    // catch up
    static auto resource =
        BoostWrap{boost::container::pmr::new_delete_resource()};

    return &resource;
}

auto Null() noexcept -> Resource*
{
    // TODO replace with std::pmr::null_memory_resource once Android and Apple
    // catch up
    static auto resource =
        BoostWrap{boost::container::pmr::null_memory_resource()};

    return &resource;
}
}  // namespace opentxs::alloc

namespace opentxs::alloc
{
Secure::Secure(Resource* upstream) noexcept
    : upstream_((nullptr == upstream) ? System() : upstream)
{
    OT_ASSERT(nullptr != upstream_);
}

auto Secure::do_allocate(std::size_t bytes, std::size_t alignment) -> void*
{
    auto* output = upstream_->allocate(bytes, alignment);

    if (nullptr == output) { throw std::bad_alloc(); }

    static auto warn{false};

    if (0 > ::sodium_mlock(output, bytes)) {
        if (false == warn) {
            LogVerbose()("Unable to lock memory. Passwords and/or secret keys "
                         "may be swapped to disk")
                .Flush();
        }

        warn = true;
    } else {
        warn = false;
    }

    return output;
}

auto Secure::do_deallocate(void* p, std::size_t size, std::size_t alignment)
    -> void
{
    ::sodium_munlock(p, size);
    upstream_->deallocate(p, size, alignment);
}

auto Secure::do_is_equal(const Resource& other) const noexcept -> bool
{
    return std::addressof(other) == this;
}

auto Secure::get() noexcept -> Resource*
{
    static auto output = Secure{System()};

    return std::addressof(output);
}
}  // namespace opentxs::alloc
