// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <atomic>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string_view>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::alloc
{
class Logging final : public Resource
{
public:
    operator Resource*() noexcept { return this; }

    auto do_is_equal(const Resource& other) const noexcept -> bool final;

    auto close() noexcept -> void;
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final;
    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final;
    auto set_name(std::string_view name) noexcept -> void;

    Logging(
        const std::filesystem::path& logfile,
        bool write,
        Resource* upstream) noexcept;
    Logging() = delete;
    Logging(const Logging&) = delete;
    Logging(Logging&&) = delete;
    auto operator=(const Logging&) -> Logging& = delete;
    auto operator=(Logging&&) -> Logging& = delete;

    ~Logging() final;

private:
    const std::filesystem::path file_;
    std::atomic<std::ptrdiff_t> total_;
    std::atomic<std::ptrdiff_t> current_;
    Resource* upstream_;
    libguarded::plain_guarded<std::ofstream> log_;
    std::atomic<bool> write_;

    template <typename Operation>
    auto write(Operation op, bool close = false) noexcept -> void;
};
}  // namespace opentxs::alloc
