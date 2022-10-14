// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/WriterPrivate.hpp"

namespace opentxs
{
Writer::Writer(WriterPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

Writer::Writer(
    std::function<WriteBuffer(std::size_t)> reserve,
    std::function<bool(std::size_t)> truncate,
    allocator_type alloc) noexcept
    : Writer([&] {
        auto pmr = alloc::PMR<WriterPrivate>(alloc);
        auto* out = pmr.allocate(1_uz);

        OT_ASSERT(nullptr != out);

        pmr.construct(out, std::move(reserve), std::move(truncate));

        return out;
    }())
{
}

Writer::Writer() noexcept
    : Writer({}, {}, {})
{
}

Writer::Writer(Writer&& rhs) noexcept
    : Writer(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto Writer::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Writer::Reserve(std::size_t val) noexcept -> WriteBuffer
{
    return imp_->Reserve(val);
}

auto Writer::Truncate(std::size_t val) noexcept -> bool
{
    return imp_->Truncate(val);
}

Writer::~Writer()
{
    if (nullptr != imp_) {
        auto pmr = alloc::PMR<WriterPrivate>(get_allocator());
        pmr.destroy(imp_);
        pmr.deallocate(imp_, 1_uz);
        imp_ = nullptr;
    }
}
}  // namespace opentxs
