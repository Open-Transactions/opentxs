// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Reader.hpp"  // IWYU pragma: associated

#include <utility>

#include "BoostIostreams.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
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
    OT_ASSERT(nullptr != imp_);
}

Reader::Reader(Reader&& rhs, allocator_type alloc) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;
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

auto Reader::swap(Reader& rhs) noexcept -> void
{
    OT_ASSERT(get_allocator() == rhs.get_allocator());

    using std::swap;
    swap(imp_, rhs.imp_);
}

Reader::~Reader()
{
    if (nullptr != imp_) {
        // TODO c++20
        auto alloc = alloc::PMR<ReaderPrivate>{get_allocator()};
        alloc.destroy(imp_);
        alloc.deallocate(imp_, 1_uz);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::storage::file
