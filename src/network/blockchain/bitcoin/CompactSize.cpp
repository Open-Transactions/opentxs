// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"

namespace be = boost::endian;

namespace opentxs::network::blockchain::bitcoin
{
auto DecodeCompactSize(ReadView& in) noexcept -> std::optional<std::size_t>
{
    auto excluded = ReadView{};

    return DecodeCompactSize(in, excluded, nullptr);
}

auto DecodeCompactSize(
    ReadView& in,
    ReadView& parsed,
    CompactSize* out) noexcept -> std::optional<std::size_t>
{
    if (in.empty()) {
        parsed = {};

        return std::nullopt;
    }

    const auto* it = reinterpret_cast<const std::byte*>(in.data());
    const auto extra = convert_to_size(CompactSize::CalculateSize(*it));
    const auto total = 1_uz + extra;
    const auto post = ScopeGuard{[&] {
        const auto safe = std::min(total, in.size());
        parsed = in.substr(0_uz, safe);
        in.remove_prefix(safe);
    }};

    if (std::byte{0} == *it) {
        if (nullptr != out) { *out = CompactSize{0u}; }

        return 0_uz;
    } else if (0_uz == extra) {
        const auto val = std::to_integer<std::size_t>(*it);

        if (nullptr != out) { *out = CompactSize{val}; }

        return val;
    } else if (in.size() >= total) {
        const auto view = in.substr(1_uz, extra);
        auto cs = std::optional<CompactSize>{std::nullopt};
        auto* effective = [&]() -> CompactSize* {
            if (nullptr == out) {

                return std::addressof(cs.emplace());
            } else {

                return out;
            }
        }();

        if (effective->Decode(view)) {
            try {

                return convert_to_size(effective->Value());
            } catch (const std::exception& e) {
                LogTrace()(__func__)(": ")(e.what()).Flush();
                // NOTE an exception can occur if decoding a CompactSize on a 32
                // bit platform because it might not be possible to represent
                // the value as a std::size_t. It is still possible to obtain
                // the output via the out argument.

                return std::nullopt;
            }
        } else {

            return std::nullopt;
        }
    } else {

        return std::nullopt;
    }
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

CompactSize::CompactSize(const ReadView bytes) noexcept(false)
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

        return 8u;
    } else if (Imp::threshold_.at(1).second == first) {

        return 4u;
    } else if (Imp::threshold_.at(0).second == first) {

        return 2u;
    } else {

        return 0u;
    }
}

template <typename SizeType>
auto CompactSize::Imp::convert_from_raw(ReadView bytes) noexcept -> void
{
    SizeType value{0u};
    std::memcpy(
        std::addressof(value),
        bytes.data(),
        std::min(sizeof(value), bytes.size()));
    be::little_to_native_inplace(value);
    data_ = value;
}

template <typename SizeType>
auto CompactSize::Imp::convert_to_raw(Writer&& output) const noexcept -> bool
{
    OT_ASSERT(std::numeric_limits<SizeType>::max() >= data_);

    auto value{static_cast<SizeType>(data_)};
    be::native_to_little_inplace(value);

    return copy(
        reader(std::addressof(value), sizeof(value)), std::move(output));
}

auto CompactSize::Decode(ReadView bytes) noexcept -> bool
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

auto CompactSize::Encode() const noexcept -> ByteArray
{
    auto out = ByteArray{};
    Encode(out.WriteInto());

    return out;
}

auto CompactSize::Encode(Writer&& destination) const noexcept -> bool
{
    auto size = Size();
    auto out = destination.Reserve(size);

    if (false == out.IsValid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output").Flush();

        return false;
    }

    auto* it = out.as<std::byte>();
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
