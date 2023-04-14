// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/work/WorkPrivate.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iterator>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain
{
WorkPrivate::WorkPrivate(Type&& data, allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
    , data_(std::move(data))
{
}

WorkPrivate::WorkPrivate(allocator_type alloc) noexcept
    : WorkPrivate(Type{}, alloc)
{
}

WorkPrivate::WorkPrivate(const WorkPrivate& rhs, allocator_type alloc) noexcept
    : WorkPrivate(Type{rhs.data_}, alloc)
{
}

auto WorkPrivate::asHex(allocator_type alloc) const noexcept -> CString
{
    auto bytes = Vector<unsigned char>{alloc};

    try {
        namespace bmp = boost::multiprecision;
        // Export as big endian
        bmp::export_bits(
            bmp::cpp_int(data_), std::back_inserter(bytes), 8, true);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to encode number").Flush();

        return {};
    }

    return to_hex(
        reinterpret_cast<const std::byte*>(bytes.data()), bytes.size(), alloc);
}

auto WorkPrivate::Decimal(allocator_type alloc) const noexcept -> CString
{
    // NOLINTNEXTLINE clang-analyzer-core.StackAddressEscape
    return {data_.str().c_str(), alloc};
}

auto WorkPrivate::IsNull() const noexcept -> bool
{
    static const auto null = Type{};

    return null == data_;
}

auto WorkPrivate::operator==(const WorkPrivate& rhs) const noexcept -> bool
{
    return data_ == rhs.data_;
}

auto WorkPrivate::operator<=>(const WorkPrivate& rhs) const noexcept
    -> std::strong_ordering
{
    if (data_ < rhs.data_) {

        return std::strong_ordering::less;
    } else if (rhs.data_ < data_) {

        return std::strong_ordering::greater;
    } else {

        return std::strong_ordering::equal;
    }
}

auto WorkPrivate::operator+(const WorkPrivate& rhs) const noexcept -> Type
{
    return data_ + rhs.data_;
}
}  // namespace opentxs::blockchain
