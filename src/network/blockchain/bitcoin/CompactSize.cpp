// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

#include "internal/util/LogMacros.hpp"
#include "network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"

namespace be = boost::endian;

namespace opentxs::network::blockchain::bitcoin
{
auto DecodeSize(
    ByteIterator& it,
    std::size_t& expected,
    const std::size_t size,
    std::size_t& output) noexcept -> bool
{
    auto cs = CompactSize{};
    auto ret = DecodeSize(it, expected, size, cs);
    output = cs.Value();

    return ret;
}

auto DecodeSize(
    ByteIterator& it,
    std::size_t& expected,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csExtraBytes) noexcept -> bool
{
    auto cs = CompactSize{};
    auto ret = DecodeSize(it, expected, size, cs);
    output = cs.Value();
    csExtraBytes = cs.Size() - 1;

    return ret;
}

auto DecodeSize(
    ByteIterator& it,
    std::size_t& expectedSize,
    const std::size_t size,
    CompactSize& output) noexcept -> bool
{
    if (std::byte{0} == *it) {
        output = CompactSize{0};
        std::advance(it, 1);
    } else {
        auto csExtraBytes = CompactSize::CalculateSize(*it);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
        // std::size_t might be 32 bit
        if (sizeof(std::size_t) < csExtraBytes) {
            LogError()("opentxs::network::blockchain::bitcoin::")(__func__)(
                ": Size too big")
                .Flush();

            return false;
        }
#pragma GCC diagnostic pop

        expectedSize += csExtraBytes;

        if (expectedSize > size) { return false; }

        if (0 == csExtraBytes) {
            output = CompactSize{std::to_integer<std::uint8_t>(*it)};
            std::advance(it, 1);
        } else {
            std::advance(it, 1);
            output = CompactSize(Space{it, it + csExtraBytes}).Value();
            std::advance(it, csExtraBytes);
        }
    }

    return true;
}

CompactSize::CompactSize() noexcept
    : imp_(std::make_unique<Imp>())
{
}

CompactSize::CompactSize(std::uint64_t value) noexcept
    : imp_(std::make_unique<Imp>(value))
{
}

CompactSize::CompactSize(const CompactSize& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
}

CompactSize::CompactSize(CompactSize&& rhs) noexcept
    : imp_(std::make_unique<Imp>())
{
    std::swap(imp_, rhs.imp_);
}

CompactSize::CompactSize(const Bytes& bytes) noexcept(false)
    : imp_(std::make_unique<Imp>())
{
    if (false == Decode(bytes)) {
        throw std::invalid_argument(
            "Wrong number of bytes: " + std::to_string(bytes.size()));
    }
}

auto CompactSize::operator=(const CompactSize& rhs) noexcept -> CompactSize&
{
    *imp_ = *rhs.imp_;

    return *this;
}

auto CompactSize::operator=(CompactSize&& rhs) noexcept -> CompactSize&
{
    if (this != &rhs) { std::swap(imp_, rhs.imp_); }

    return *this;
}

auto CompactSize::operator=(const std::uint64_t rhs) noexcept -> CompactSize&
{
    imp_->data_ = rhs;

    return *this;
}

auto CompactSize::CalculateSize(const std::byte first) noexcept -> std::uint64_t
{
    if (Imp::threshold_.at(2).second == first) {

        return 8;
    } else if (Imp::threshold_.at(1).second == first) {

        return 4;
    } else if (Imp::threshold_.at(0).second == first) {

        return 2;
    } else {

        return 0;
    }
}

template <typename SizeType>
void CompactSize::Imp::convert_from_raw(
    const std::pmr::vector<std::byte>& bytes) noexcept
{
    SizeType value{0};
    std::memcpy(&value, bytes.data(), sizeof(value));
    be::little_to_native_inplace(value);
    data_ = value;
}

template <typename SizeType>
auto CompactSize::Imp::convert_to_raw(AllocateOutput output) const noexcept
    -> bool
{
    OT_ASSERT(std::numeric_limits<SizeType>::max() >= data_);

    if (false == bool(output)) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    const auto out = output(sizeof(SizeType));

    if (false == out.valid(sizeof(SizeType))) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output").Flush();

        return false;
    }

    auto value{static_cast<SizeType>(data_)};
    be::native_to_little_inplace(value);
    std::memcpy(out.data(), &value, sizeof(value));

    return true;
}

auto CompactSize::Decode(const std::pmr::vector<std::byte>& bytes) noexcept
    -> bool
{
    bool output{true};

    if (sizeof(std::uint8_t) == bytes.size()) {
        imp_->convert_from_raw<std::uint8_t>(bytes);
    } else if (sizeof(std::uint16_t) == bytes.size()) {
        imp_->convert_from_raw<std::uint16_t>(bytes);
    } else if (sizeof(std::uint32_t) == bytes.size()) {
        imp_->convert_from_raw<std::uint32_t>(bytes);
    } else if (sizeof(std::uint64_t) == bytes.size()) {
        imp_->convert_from_raw<std::uint64_t>(bytes);
    } else {
        LogError()(OT_PRETTY_CLASS())("Wrong number of bytes: ")(bytes.size())
            .Flush();
        output = false;
    }

    return output;
}

auto CompactSize::Encode() const noexcept -> std::pmr::vector<std::byte>
{
    auto output = Space{};
    Encode(writer(output));

    return output;
}

auto CompactSize::Encode(AllocateOutput destination) const noexcept -> bool
{
    if (false == bool(destination)) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    auto size = Size();
    const auto out = destination(size);

    if (false == out.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output").Flush();

        return false;
    }

    auto it = static_cast<std::byte*>(out.data());
    const auto& data = imp_->data_;

    if (data <= Imp::threshold_.at(0).first) {
        imp_->convert_to_raw<std::uint8_t>(preallocated(size, it));
    } else if (data <= Imp::threshold_.at(1).first) {
        *it = Imp::threshold_.at(0).second;
        std::advance(it, 1);
        size -= 1;
        imp_->convert_to_raw<std::uint16_t>(preallocated(size, it));
    } else if (data <= Imp::threshold_.at(2).first) {
        *it = Imp::threshold_.at(1).second;
        std::advance(it, 1);
        size -= 1;
        imp_->convert_to_raw<std::uint32_t>(preallocated(size, it));
    } else {
        *it = Imp::threshold_.at(2).second;
        std::advance(it, 1);
        size -= 1;
        imp_->convert_to_raw<std::uint64_t>(preallocated(size, it));
    }

    return true;
}

auto CompactSize::Size() const noexcept -> std::size_t
{
    const auto& data = imp_->data_;

    if (data <= Imp::threshold_.at(0).first) {

        return 1;
    } else if (data <= Imp::threshold_.at(1).first) {

        return 3;
    } else if (data <= Imp::threshold_.at(2).first) {

        return 5;
    } else {

        return 9;
    }
}

auto CompactSize::Total() const noexcept -> std::size_t
{
    return Size() + static_cast<std::size_t>(imp_->data_);
}

auto CompactSize::Value() const noexcept -> std::uint64_t
{
    return imp_->data_;
}

CompactSize::~CompactSize() = default;
}  // namespace opentxs::network::blockchain::bitcoin
