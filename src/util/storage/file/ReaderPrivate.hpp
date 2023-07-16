// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>

#include "BoostIostreams.hpp"
#include "internal/util/PMR.hpp"
#include "util/Allocated.hpp"

namespace opentxs::storage::file
{
class ReaderPrivate final : public opentxs::implementation::Allocated
{
public:
    const boost::iostreams::mapped_file_source file_;

    auto clone(allocator_type alloc) const noexcept -> ReaderPrivate*
    {
        return pmr::clone(this, {alloc});
    }
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    ReaderPrivate(
        std::filesystem::path file,
        std::size_t offset,
        std::size_t length,
        allocator_type alloc) noexcept;
    ReaderPrivate() = delete;
    ReaderPrivate(const ReaderPrivate& rhs, allocator_type alloc = {}) noexcept;
    ReaderPrivate(ReaderPrivate&&) = delete;
    auto operator=(const ReaderPrivate&) noexcept -> ReaderPrivate& = delete;
    auto operator=(ReaderPrivate&& rhs) noexcept -> ReaderPrivate& = delete;

    ~ReaderPrivate() final = default;
};
}  // namespace opentxs::storage::file
