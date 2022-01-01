// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "blockchain/Work.hpp"  // IWYU pragma: associated

#include <boost/exception/exception.hpp>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto Work(const std::string& hex) -> blockchain::Work*
{
    using ReturnType = blockchain::implementation::Work;
    using ValueType = ReturnType::Type;

    const auto bytes = Data::Factory(hex, Data::Mode::Hex);

    if (bytes->empty()) { return new ReturnType(); }

    ValueType value{};
    bmp::cpp_int i;

    try {
        // Interpret bytes as big endian
        bmp::import_bits(i, bytes->begin(), bytes->end(), 8, true);
        value = ValueType{i};
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Failed to decode work")
            .Flush();

        return new ReturnType();
    }

    return new ReturnType(std::move(value));
}

auto Work(const blockchain::Type chain, const blockchain::NumericHash& input)
    -> blockchain::Work*
{
    using ReturnType = blockchain::implementation::Work;
    using TargetType = bmp::checked_cpp_int;
    using ValueType = ReturnType::Type;

    auto value = ValueType{};

    try {
        const auto maxTarget = factory::NumericHashNBits(
            blockchain::NumericHash::MaxTarget(chain));

        OT_ASSERT(maxTarget);

        const auto max = TargetType{maxTarget->Decimal()};
        const auto incoming = TargetType{input.Decimal()};

        if (incoming > max) {
            value = ValueType{1};
        } else {
            value = ValueType{max} / ValueType{incoming};
        }
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to calculate difficulty")
            .Flush();

        return new ReturnType();
    }

    return new ReturnType(std::move(value));
}
}  // namespace opentxs::factory

namespace opentxs
{
auto operator==(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() == rhs;
}

auto operator!=(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() != rhs;
}

auto operator<(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() < rhs;
}

auto operator<=(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() <= rhs;
}

auto operator>(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() > rhs;
}

auto operator>=(const OTWork& lhs, const blockchain::Work& rhs) noexcept -> bool
{
    return lhs.get() >= rhs;
}

auto operator+(const OTWork& lhs, const blockchain::Work& rhs) noexcept
    -> OTWork
{
    return lhs.get() + rhs;
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
Work::Work(Type&& data) noexcept
    : blockchain::Work()
    , data_(std::move(data))
{
}

Work::Work() noexcept
    : Work(Type{})
{
}

Work::Work(const Work& rhs) noexcept
    : Work(Type{rhs.data_})
{
}

auto Work::operator==(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ == input.data_;
}

auto Work::operator!=(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ != input.data_;
}

auto Work::operator<(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ < input.data_;
}

auto Work::operator<=(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ <= input.data_;
}

auto Work::operator>(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ > input.data_;
}

auto Work::operator>=(const blockchain::Work& rhs) const noexcept -> bool
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return data_ >= input.data_;
}

auto Work::operator+(const blockchain::Work& rhs) const noexcept -> OTWork
{
    const auto& input = dynamic_cast<const Work&>(rhs);

    return OTWork{new Work{data_ + input.data_}};
}

auto Work::asHex() const noexcept -> std::string
{
    std::pmr::vector<unsigned char> bytes;

    try {
        // Export as big endian
        bmp::export_bits(
            bmp::cpp_int(data_), std::back_inserter(bytes), 8, true);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to encode number").Flush();

        return {};
    }

    return opentxs::Data::Factory(bytes.data(), bytes.size())->asHex();
}
}  // namespace opentxs::blockchain::implementation
