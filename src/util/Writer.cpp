// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cstddef>
#include <functional>
#include <utility>

#include "internal/util/PMR.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/WriterPrivate.hpp"

namespace opentxs
{
Writer::Writer(WriterPrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);
}

Writer::Writer(
    std::function<WriteBuffer(std::size_t)> reserve,
    std::function<bool(std::size_t)> truncate,
    allocator_type alloc) noexcept
    : Writer(pmr::construct<WriterPrivate>(
          alloc,
          std::move(reserve),
          std::move(truncate)))
{
}

Writer::Writer() noexcept
    : Writer({}, {}, {})
{
}

Writer::Writer(Writer&& rhs) noexcept
    : Writer(std::exchange(rhs.imp_, nullptr))
{
}

Writer::Writer(Writer&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto Writer::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Writer::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

auto Writer::Reserve(std::size_t val) noexcept -> WriteBuffer
{
    return imp_->Reserve(val);
}

auto Writer::Truncate(std::size_t val) noexcept -> bool
{
    return imp_->Truncate(val);
}

Writer::~Writer() { pmr::destroy(imp_); }
}  // namespace opentxs
