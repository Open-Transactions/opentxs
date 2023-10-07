// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Reader.hpp"  // IWYU pragma: associated

#include <utility>

#include "BoostIostreams.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/file/ReaderPrivate.hpp"

namespace opentxs::storage::file
{
auto swap(Reader& lhs, Reader& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::storage::file

namespace opentxs::storage::file
{
Reader::Reader(ReaderPrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Reader::Reader(Reader&& rhs) noexcept
    : Reader(std::exchange(rhs.imp_, nullptr))
{
}

Reader::Reader(Reader&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Reader::get() const noexcept -> ReadView
{
    const auto& mmap = imp_->file_;

    return {mmap.data(), mmap.size()};
}

auto Reader::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Reader::swap(Reader& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

Reader::~Reader() { pmr::destroy(imp_); }
}  // namespace opentxs::storage::file
