// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>

#include "internal/util/storage/file/Reader.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage::file
{
struct Position {
    std::optional<std::filesystem::path> file_name_{std::nullopt};
    std::size_t offset_{};
    std::size_t length_{};

    operator bool() const noexcept { return IsValid(); }

    auto IsValid() const noexcept -> bool { return file_name_.has_value(); }
};

auto Read(std::span<const Position> in, alloc::Default alloc) noexcept
    -> Vector<Reader>;
auto Read(const Position& in, alloc::Default alloc) noexcept -> Reader;
}  // namespace opentxs::storage::file
