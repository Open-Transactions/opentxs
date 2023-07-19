// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/block/NumericHashPrivate.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::block
{
NumericHashPrivate::NumericHashPrivate(Type&& data) noexcept
    : data_(std::move(data))
{
}

NumericHashPrivate::NumericHashPrivate() noexcept
    : NumericHashPrivate(Type{})
{
}

NumericHashPrivate::NumericHashPrivate(std::uint32_t input) noexcept
    : NumericHashPrivate([&]() -> Type {
        auto target = Type{};
        const auto mantissa = std::uint32_t{input & 0x007fffff};
        const auto exponent =
            static_cast<std::uint8_t>((input & 0xff000000) >> 24);

        try {
            if (exponent > 3) {
                target = Type{mantissa} << (8 * (exponent - 3));
            } else {
                target = Type{mantissa} << (8 * (3 - exponent));
            }

            return target;
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to calculate target").Flush();

            return {};
        }
    }())
{
}

NumericHashPrivate::NumericHashPrivate(const Data& data) noexcept
    : NumericHashPrivate([&]() -> Type {
        auto value = Type{};

        if (data.IsNull()) { return {}; }

        try {
            auto d = data.get();
            // Interpret hash as little endian
            boost::multiprecision::import_bits(
                value, d.begin(), d.end(), 8, false);

            return value;
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("failed to decode hash").Flush();

            return {};
        }
    }())
{
}

NumericHashPrivate::NumericHashPrivate(const NumericHashPrivate& rhs) noexcept
    : NumericHashPrivate(Type{rhs.data_})
{
}

auto NumericHashPrivate::asHex(const std::size_t minimumBytes) const noexcept
    -> UnallocatedCString
{
    auto bytes = UnallocatedVector<unsigned char>{};

    try {
        // Export as big endian
        boost::multiprecision::export_bits(
            data_, std::back_inserter(bytes), 8, true);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to encode number").Flush();

        return {};
    }

    while (minimumBytes > bytes.size()) { bytes.insert(bytes.begin(), 0x0); }

    return ByteArray{bytes.data(), bytes.size()}.asHex();
}

auto NumericHashPrivate::Decimal() const noexcept -> UnallocatedCString
{
    return data_.str();
}

auto NumericHashPrivate::operator<=>(
    const NumericHashPrivate& rhs) const noexcept -> std::strong_ordering
{
    if (data_ < rhs.data_) {

        return std::strong_ordering::less;
    } else if (rhs.data_ < data_) {

        return std::strong_ordering::greater;
    } else {

        return std::strong_ordering::equal;
    }
}

auto NumericHashPrivate::operator==(
    const NumericHashPrivate& rhs) const noexcept -> bool
{
    return data_ == rhs.data_;
}
}  // namespace opentxs::blockchain::block
