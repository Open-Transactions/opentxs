// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/Work.hpp"  // IWYU pragma: associated

#include "internal/util/PMR.hpp"

namespace opentxs::blockchain
{
auto swap(Work& lhs, Work& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::blockchain

namespace opentxs::blockchain
{
Work::Work(WorkPrivate*) noexcept
    : imp_(nullptr)
{
}

Work::Work(allocator_type) noexcept
    : imp_(nullptr)
{
}

Work::Work(const block::NumericHash&, blockchain::Type, allocator_type) noexcept
    : imp_(nullptr)
{
}

Work::Work(const HexType&, std::string_view, allocator_type) noexcept
    : imp_(nullptr)
{
}

Work::Work(const Work&, allocator_type) noexcept
    : imp_(nullptr)
{
}

Work::Work(Work&& rhs) noexcept
    : imp_(nullptr)
{
}

Work::Work(Work&&, allocator_type) noexcept
    : imp_(nullptr)
{
}

auto Work::asHex(allocator_type) const noexcept -> CString { return {}; }

auto Work::asHex() const noexcept -> UnallocatedCString { return {}; }

auto Work::Decimal(allocator_type) const noexcept -> CString { return {}; }

auto Work::Decimal() const noexcept -> UnallocatedCString { return {}; }

auto Work::get_allocator() const noexcept -> allocator_type { return {}; }

auto Work::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Work::IsNull() const noexcept -> bool { return {}; }

auto Work::operator=(const Work&) noexcept -> Work& { return *this; }

auto Work::operator<=>(const blockchain::Work& rhs) const noexcept
    -> std::strong_ordering
{
    return std::strong_ordering::equal;
}

auto Work::operator==(const blockchain::Work& rhs) const noexcept -> bool
{
    return true;
}

auto Work::operator+(const Work& rhs) const noexcept -> Work { return {}; }

auto Work::swap(Work& rhs) noexcept -> void {}

auto Work::operator=(Work&& rhs) noexcept -> Work& { return *this; }

Work::~Work() = default;
}  // namespace opentxs::blockchain
