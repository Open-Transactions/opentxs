// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "core/identifier/Imp.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <Identifier.pb.h>
#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <sstream>
#include <utility>

#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identifier
{
Generic::Imp::Imp(
    const identifier::Algorithm algorithm,
    const identifier::Type type,
    const ReadView hash,
    allocator_type alloc) noexcept
    : algorithm_(algorithm)
    , type_(type)
    , parent_(nullptr)
    , data_(alloc)
{
    Concatenate(hash);
}

auto Generic::Imp::asBase58(const api::Crypto& api) const -> UnallocatedCString
{
    return asBase58(api, {}).c_str();
}

auto Generic::Imp::asBase58(const api::Crypto& api, alloc::Default alloc) const
    -> CString
{
    const auto required = identifier_expected_hash_bytes(algorithm_);

    if (const auto len = size(); len != required) {
        if (0u != len) {
            LogError()(OT_PRETTY_CLASS())("Incorrect hash size (")(
                len)(") vs required (")(required)(")")
                .Flush();
        }

        return CString{alloc};
    }

    const auto preimage = [&] {
        auto out = ByteArray{alloc};
        const auto payload = size();

        if (0 == payload) { return out; }

        const auto type = boost::endian::little_uint16_buf_t{
            static_cast<std::uint16_t>(type_)};
        out.resize(sizeof(algorithm_) + sizeof(type) + payload);

        OT_ASSERT(out.size() == required + identifier_header_);

        auto* i = static_cast<std::byte*>(out.data());
        std::memcpy(i, &algorithm_, sizeof(algorithm_));
        std::advance(i, sizeof(algorithm_));
        std::memcpy(i, static_cast<const void*>(&type), sizeof(type));
        std::advance(i, sizeof(type));
        std::memcpy(i, data(), payload);
        std::advance(i, payload);

        return out;
    }();
    // TODO c++20 use allocator
    auto ss = std::stringstream{};

    if (0 < preimage.size()) {
        ss << identifier_prefix_;
        ss << api.Encode().IdentifierEncode(preimage.Bytes());
    }

    return CString{ss.str().c_str(), alloc};
}

auto Generic::Imp::asHex() const -> UnallocatedCString
{
    return to_hex(
        reinterpret_cast<const std::byte*>(data_.data()), data_.size());
}

auto Generic::Imp::asHex(alloc::Default alloc) const -> CString
{
    return to_hex(
        reinterpret_cast<const std::byte*>(data_.data()), data_.size(), alloc);
}

auto Generic::Imp::Assign(const void* data, const std::size_t size) noexcept
    -> bool
{
    auto rhs = [&]() -> Vector {
        if ((data == nullptr) || (size == 0_uz)) {

            return {};
        } else {
            const auto* i = static_cast<const std::byte*>(data);

            return {i, std::next(i, size)};
        }
    }();
    data_.swap(rhs);

    return true;
}

auto Generic::Imp::begin() -> iterator { return {parent_, 0}; }

auto Generic::Imp::cbegin() const -> const_iterator { return {parent_, 0}; }

auto Generic::Imp::cend() const -> const_iterator
{
    return {parent_, data_.size()};
}

auto Generic::Imp::check_sub(const std::size_t pos, const std::size_t target)
    const -> bool
{
    return check_subset(data_.size(), target, pos);
}

auto Generic::Imp::concatenate(const Vector& data) -> void
{
    data_.insert(data_.end(), data.begin(), data.end());
}

auto Generic::Imp::Concatenate(
    const void* data,
    const std::size_t size) noexcept -> bool
{
    if ((size == 0_uz) || (nullptr == data)) { return false; }

    const auto* ptr = static_cast<const std::byte*>(data);
    data_.insert(data_.end(), ptr, std::next(ptr, size));

    return true;
}

auto Generic::Imp::DecodeHex(const std::string_view hex) -> bool
{
    data_.clear();

    if (hex.empty()) { return true; }

    if (2 > hex.size()) { return false; }

    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    using namespace std::literals;
    // TODO c++20 use ranges to prevent unnecessary copy
    const auto padded = (0 == stripped.size() % 2)
                            ? CString{stripped}
                            : CString{"0"sv}.append(stripped);

    for (std::size_t i = 0; i < padded.length(); i += 2) {
        data_.emplace_back(std::byte(static_cast<std::uint8_t>(
            strtol(padded.substr(i, 2).c_str(), nullptr, 16))));
    }

    return true;
}

auto Generic::Imp::end() -> iterator { return {parent_, data_.size()}; }

auto Generic::Imp::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

auto Generic::Imp::Extract(std::uint8_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    output = std::to_integer<std::uint8_t>(data_.at(pos));

    return true;
}

auto Generic::Imp::Extract(std::uint16_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint16_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto Generic::Imp::Extract(std::uint32_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint32_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto Generic::Imp::Extract(std::uint64_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint64_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto Generic::Imp::IsNull() const -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        static constexpr auto null = std::byte{0x0};

        if (null != byte) { return false; }
    }

    return true;
}

auto Generic::Imp::operator+=(const opentxs::Data& rhs) noexcept -> Generic&
{
    Concatenate(rhs.data(), rhs.size());

    return *parent_;
}

auto Generic::Imp::operator+=(const ReadView rhs) noexcept -> Generic&
{
    Concatenate(rhs);

    return *parent_;
}

auto Generic::Imp::operator+=(const std::uint8_t rhs) noexcept -> Generic&
{
    data_.emplace_back(std::byte(rhs));

    return *parent_;
}

auto Generic::Imp::operator+=(const std::uint16_t rhs) noexcept -> Generic&
{
    const auto input = boost::endian::big_uint16_buf_t(rhs);
    const auto* ptr = reinterpret_cast<const std::byte*>(std::addressof(input));
    data_.insert(data_.end(), ptr, std::next(ptr, sizeof(input)));

    return *parent_;
}

auto Generic::Imp::operator+=(const std::uint32_t rhs) noexcept -> Generic&
{
    const auto input = boost::endian::big_uint32_buf_t(rhs);
    const auto* ptr = reinterpret_cast<const std::byte*>(std::addressof(input));
    data_.insert(data_.end(), ptr, std::next(ptr, sizeof(input)));

    return *parent_;
}

auto Generic::Imp::operator+=(const std::uint64_t rhs) noexcept -> Generic&
{
    const auto input = boost::endian::big_uint64_buf_t(rhs);
    const auto* ptr = reinterpret_cast<const std::byte*>(std::addressof(input));
    data_.insert(data_.end(), ptr, std::next(ptr, sizeof(input)));

    return *parent_;
}

auto Generic::Imp::Randomize(const std::size_t size) -> bool
{
    SetSize(size);

    if (size == 0_uz) { return false; }

    ::randombytes_buf(data_.data(), size);

    return true;
}

auto Generic::Imp::resize(const std::size_t size) -> bool
{
    data_.resize(size);

    return true;
}

auto Generic::Imp::Serialize(proto::Identifier& out) const noexcept -> bool
{
    out.set_version(proto_version_);
    static constexpr auto badAlgo = identifier::Algorithm::invalid;
    static constexpr auto badType = identifier::Type::invalid;

    if ((badAlgo == algorithm_) || (badType == type_)) {
        out.clear_hash();
        out.set_algorithm(static_cast<std::uint32_t>(badAlgo));
        out.set_type(static_cast<std::uint32_t>(badType));

        return true;
    }

    out.set_hash(UnallocatedCString{Bytes()});
    out.set_algorithm(static_cast<std::uint32_t>(algorithm_));
    out.set_type(static_cast<std::uint32_t>(type_));

    return true;
}

auto Generic::Imp::SetSize(const std::size_t size) -> bool
{
    clear();

    if (size > 0) { data_.assign(size, std::byte{}); }

    return true;
}

auto Generic::Imp::WriteInto() noexcept -> AllocateOutput
{
    return [this](const auto size) {
        static constexpr auto blank = std::byte{51};
        data_.clear();
        data_.assign(size, blank);

        return WritableView{data_.data(), data_.size()};
    };
}

auto Generic::Imp::zeroMemory() -> void
{
    ::sodium_memzero(data_.data(), data_.size());
}
}  // namespace opentxs::identifier
