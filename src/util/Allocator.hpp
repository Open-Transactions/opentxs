// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocator.hpp"

#include <cstddef>

namespace opentxs::alloc
{
class Secure final : public Resource
{
public:
    static auto get() noexcept -> Resource*;

    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final;
    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final;
    auto do_is_equal(const Resource& other) const noexcept -> bool final;

    Secure() = delete;
    Secure(const Secure&) = delete;
    Secure(Secure&&) = delete;
    auto operator=(const Secure&) -> Secure& = delete;
    auto operator=(Secure&&) -> Secure& = delete;

    ~Secure() final = default;

private:
    Resource* upstream_;

    Secure(Resource* upstream) noexcept;
};
}  // namespace opentxs::alloc
