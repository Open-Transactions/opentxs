// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sodium.h>
}

#include <cstddef>
#include <cstdlib>
#include <limits>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <typename T>
struct SecureAllocator {
    using value_type = T;

    SecureAllocator()
    {
        if (0 > ::sodium_init()) { throw std::bad_alloc(); }
    }

    template <class U>
    SecureAllocator(const SecureAllocator<U>&)
    {
        if (0 > ::sodium_init()) { throw std::bad_alloc(); }
    }

    auto allocate(const std::size_t items) -> value_type*
    {
        constexpr auto limit =
            std::numeric_limits<std::size_t>::max() / sizeof(value_type);

        if (items > limit) { throw std::bad_alloc(); }

        const auto bytes = items * sizeof(value_type);
        auto output = std::malloc(bytes);

        if (nullptr == output) { throw std::bad_alloc(); }

        static auto warn{false};

        if (0 > ::sodium_mlock(output, bytes)) {
            if (false == warn) {
                LogVerbose()("Unable to lock memory. Passwords and/or secret "
                             "keys may be swapped to disk")
                    .Flush();
            }

            warn = true;
        } else {
            warn = false;
        }

        return static_cast<value_type*>(output);
    }
    auto deallocate(value_type* in, const std::size_t bytes) -> void
    {
        ::sodium_munlock(in, bytes);
        std::free(in);
    }
};

template <typename T, typename U>
auto operator==(const SecureAllocator<T>&, const SecureAllocator<U>&) noexcept
    -> bool
{
    return true;
}

template <typename T, typename U>
auto operator!=(
    const SecureAllocator<T>& lhs,
    const SecureAllocator<U>& rhs) noexcept -> bool
{
    return !(lhs == rhs);
}
}  // namespace opentxs
