// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "core/Data.hpp"   // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <limits>
#include <memory_resource>
#include <sstream>
#include <utility>

#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::Data>;

namespace std
{
auto less<opentxs::Pimpl<opentxs::Data>>::operator()(
    const opentxs::OTData& lhs,
    const opentxs::OTData& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs
{
auto operator==(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() == rhs;
}

auto operator!=(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() != rhs;
}

auto operator<(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() < rhs;
}

auto operator>(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() > rhs;
}

auto operator<=(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() <= rhs;
}

auto operator>=(const OTData& lhs, const Data& rhs) noexcept -> bool
{
    return lhs.get() >= rhs;
}

auto operator+=(OTData& lhs, const OTData& rhs) -> OTData&
{
    lhs.get() += rhs.get();

    return lhs;
}

auto operator+=(OTData& lhs, const std::uint8_t rhs) -> OTData&
{
    lhs.get() += rhs;

    return lhs;
}

auto operator+=(OTData& lhs, const std::uint16_t rhs) -> OTData&
{
    lhs.get() += rhs;

    return lhs;
}

auto operator+=(OTData& lhs, const std::uint32_t rhs) -> OTData&
{
    lhs.get() += rhs;

    return lhs;
}

auto operator+=(OTData& lhs, const std::uint64_t rhs) -> OTData&
{
    lhs.get() += rhs;

    return lhs;
}

auto Data::Factory() -> OTData { return OTData(new implementation::Data()); }

auto Data::Factory(const Data& rhs) -> OTData
{
    return OTData(new implementation::Data(rhs.data(), rhs.size()));
}

auto Data::Factory(const void* data, std::size_t size) -> OTData
{
    return OTData(new implementation::Data(data, size));
}

auto Data::Factory(const Armored& source) -> OTData
{
    return OTData(new implementation::Data(source));
}

auto Data::Factory(const std::pmr::vector<unsigned char>& source) -> OTData
{
    return OTData(new implementation::Data(source));
}

auto Data::Factory(const std::pmr::vector<std::byte>& source) -> OTData
{
    return OTData(new implementation::Data(source));
}

auto Data::Factory(const network::zeromq::Frame& message) -> OTData
{
    return OTData(new implementation::Data(message.data(), message.size()));
}

auto Data::Factory(const std::uint8_t in) -> OTData
{
    return OTData(new implementation::Data(&in, sizeof(in)));
}

auto Data::Factory(const std::uint16_t in) -> OTData
{
    const auto input = boost::endian::big_uint16_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

auto Data::Factory(const std::uint32_t in) -> OTData
{
    const auto input = boost::endian::big_uint32_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

auto Data::Factory(const std::uint64_t in) -> OTData
{
    const auto input = boost::endian::big_uint64_buf_t(in);

    return OTData(new implementation::Data(&input, sizeof(input)));
}

auto Data::Factory(const std::string in, const Mode mode) -> OTData
{
    if (Mode::Hex == mode) {
        auto output = OTData(new implementation::Data(in.data(), in.size()));

        if (output->DecodeHex(in)) { return output; }

        return OTData(new implementation::Data());
    } else {
        return OTData(new implementation::Data(in.data(), in.size()));
    }
}

namespace implementation
{
Data::Data() noexcept
    : data_()
{
}

Data::Data(const Armored& source) noexcept
    : data_()
{
    if (source.Exists()) { source.GetData(*this); }
}

Data::Data(const void* data, std::size_t size) noexcept
    : data_(
          static_cast<const std::uint8_t*>(data),
          static_cast<const std::uint8_t*>(data) + size)
{
}

Data::Data(const Vector& v) noexcept
    : Data(Vector(v))
{
}

Data::Data(Vector&& v) noexcept
    : data_(std::move(v))
{
}

Data::Data(const std::pmr::vector<std::byte>& v) noexcept
    : data_(
          reinterpret_cast<const std::uint8_t*>(v.data()),
          reinterpret_cast<const std::uint8_t*>(v.data()) + v.size())
{
}

auto Data::operator==(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 == spaceship(rhs);
}

auto Data::operator!=(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 != spaceship(rhs);
}

auto Data::operator<(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 > spaceship(rhs);
}

auto Data::operator>(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 < spaceship(rhs);
}

auto Data::operator<=(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 >= spaceship(rhs);
}

auto Data::operator>=(const opentxs::Data& rhs) const noexcept -> bool
{
    return 0 <= spaceship(rhs);
}

auto Data::operator+=(const opentxs::Data& rhs) -> Data&
{
    concatenate(dynamic_cast<const Data&>(rhs).data_);

    return *this;
}

auto Data::operator+=(const std::uint8_t rhs) -> Data&
{
    data_.emplace_back(rhs);

    return *this;
}

auto Data::operator+=(const std::uint16_t rhs) -> Data&
{
    const auto input = boost::endian::big_uint16_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

auto Data::operator+=(const std::uint32_t rhs) -> Data&
{
    const auto input = boost::endian::big_uint32_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

auto Data::operator+=(const std::uint64_t rhs) -> Data&
{
    const auto input = boost::endian::big_uint64_buf_t(rhs);
    Data temp(&input, sizeof(input));
    concatenate(temp.data_);

    return *this;
}

auto Data::asHex() const -> std::string
{
    std::stringstream out{};

    // TODO: std::to_integer<int>(byte)

    for (const auto byte : data_) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<const int&>(byte);
    }

    return out.str();
}

auto Data::Assign(const void* data, const std::size_t size) noexcept -> bool
{
    auto rhs = [&]() -> Vector {
        if ((data == nullptr) || (size == 0)) {

            return {};
        } else {
            const auto* i = static_cast<const std::uint8_t*>(data);

            return {i, std::next(i, size)};
        }
    }();
    data_.swap(rhs);

    return true;
}

auto Data::check_sub(const std::size_t pos, const std::size_t target) const
    -> bool
{
    const auto size = data_.size();

    if (pos > size) { return false; }

    if ((std::numeric_limits<std::size_t>::max() - pos) < target) {
        return false;
    }

    if ((pos + target) > size) { return false; }

    return true;
}

void Data::concatenate(const Vector& data)
{
    for (const auto& byte : data) {
        data_.emplace_back(reinterpret_cast<const std::uint8_t&>(byte));
    }
}

auto Data::Concatenate(const void* data, const std::size_t size) noexcept
    -> bool
{
    if ((size == 0) || (nullptr == data)) { return false; }

    Data temp(data, size);
    concatenate(temp.data_);

    return true;
}

auto Data::DecodeHex(const std::string& hex) -> bool
{
    data_.clear();

    if (hex.empty()) { return true; }

    if (2 > hex.size()) { return false; }

    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    const auto padded =
        (0 == stripped.size() % 2) ? stripped : std::string("0") + stripped;

    for (std::size_t i = 0; i < padded.length(); i += 2) {
        data_.emplace_back(static_cast<std::uint8_t>(
            strtol(padded.substr(i, 2).c_str(), nullptr, 16)));
    }

    return true;
}

auto Data::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

auto Data::Extract(std::uint8_t& output, const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    output = data_.at(pos);

    return true;
}

auto Data::Extract(std::uint16_t& output, const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint16_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto Data::Extract(std::uint32_t& output, const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint32_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto Data::Extract(std::uint64_t& output, const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint64_buf_t();
    std::memcpy(&temp, &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

void Data::Initialize() { data_.clear(); }

auto Data::IsNull() const -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        if (0 != byte) { return false; }
    }

    return true;
}

auto Data::Randomize(const std::size_t size) -> bool
{
    SetSize(size);

    if (size == 0) { return false; }

    ::randombytes_buf(data_.data(), size);

    return true;
}

void Data::Release()
{
    zeroMemory();
    Initialize();
}

void Data::SetSize(const std::size_t size)
{
    Release();

    if (size > 0) { data_.assign(size, 0); }
}

auto Data::spaceship(const opentxs::Data& rhs) const noexcept -> int
{
    const auto lSize = data_.size();
    const auto rSize = rhs.size();

    if ((0u == lSize) && (0u == rSize)) { return 0; }
    if (lSize < rSize) { return -1; }
    if (lSize > rSize) { return 1; }

    return std::memcmp(data_.data(), rhs.data(), data_.size());
}

auto Data::str() const -> std::string
{
    if (data_.empty()) { return {}; }

    return std::string{reinterpret_cast<const char*>(&data_[0]), data_.size()};
}

void Data::swap(opentxs::Data&& rhs)
{
    auto& in = dynamic_cast<Data&>(rhs);
    std::swap(data_, in.data_);
}

auto Data::WriteInto() noexcept -> AllocateOutput
{
    return [this](const auto size) {
        data_.clear();
        data_.assign(size, 51);

        return WritableView{data_.data(), data_.size()};
    };
}

void Data::zeroMemory()
{
    if (0 < data_.size()) { data_.assign(data_.size(), 0); }
}
}  // namespace implementation
}  // namespace opentxs
