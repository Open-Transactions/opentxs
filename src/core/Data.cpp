// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"

#include <cstdio>
#include <iomanip>
#include <sstream>

#include "Data.hpp"

template class opentxs::Pimpl<opentxs::Data>;

namespace opentxs
{
bool operator==(OTData& lhs, const Data& rhs) { return lhs.get() == rhs; }

bool operator!=(OTData& lhs, const Data& rhs) { return lhs.get() != rhs; }

OTData& operator+=(OTData& lhs, const OTData& rhs)
{
    lhs.get() += rhs.get();

    return lhs;
}

OTData Data::Factory() { return OTData(new implementation::Data()); }

OTData Data::Factory(const Data& rhs)
{
    return OTData(new implementation::Data(rhs.data(), rhs.size()));
}

OTData Data::Factory(const void* data, std::size_t size)
{
    return OTData(new implementation::Data(data, size));
}

OTData Data::Factory(const Armored& source)
{
    return OTData(new implementation::Data(source));
}

OTData Data::Factory(const std::vector<unsigned char>& source)
{
    return OTData(new implementation::Data(source));
}

OTData Data::Factory(const network::zeromq::Frame& message)
{
    return OTData(new implementation::Data(message.data(), message.size()));
}

namespace implementation
{
Data::Data(const Armored& source)
{
    if (source.Exists()) { source.GetData(*this); }
}

Data::Data(const void* data, std::size_t size)
    : data_(
          static_cast<const std::uint8_t*>(data),
          static_cast<const std::uint8_t*>(data) + size)
{
}

Data::Data(const std::vector<unsigned char>& sourceVector)
{
    Assign(sourceVector.data(), sourceVector.size());
}

Data::Data(const Vector& rhs, const std::size_t size)
    : data_{rhs}
    , position_{size}
{
}

bool Data::operator==(const opentxs::Data& rhs) const
{
    return data_ == dynamic_cast<const Data&>(rhs).data_;
}

bool Data::operator!=(const opentxs::Data& rhs) const
{
    return !operator==(rhs);
}

Data& Data::operator+=(const opentxs::Data& rhs)
{
    concatenate(dynamic_cast<const Data&>(rhs).data_);

    return *this;
}

std::string Data::asHex() const
{
    std::stringstream out{};

    // TODO: std::to_integer<int>(byte)

    for (const auto byte : data_) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<const int&>(byte);
    }

    return out.str();
}

void Data::Assign(const opentxs::Data& rhs)
{
    // can't assign to self.
    if (&dynamic_cast<const Data&>(rhs) == this) { return; }

    data_ = dynamic_cast<const Data&>(rhs).data_;
    position_ = dynamic_cast<const Data&>(rhs).position_;
}

void Data::Assign(const void* data, const std::size_t& size)
{
    Release();

    if (data != nullptr && size > 0) {
        auto start = static_cast<const std::uint8_t*>(data);
        const std::uint8_t* end = start + size;
        data_.assign(start, end);
    }
}

void Data::concatenate(const Vector& data)
{
    for (const auto& byte : data) { data_.emplace_back(byte); }
}

void Data::Concatenate(const void* data, const std::size_t& size)
{
    OT_ASSERT(data != nullptr);
    OT_ASSERT(size > 0);

    if (size == 0) { return; }

    Data temp(data, size);
    concatenate(temp.data_);
}

void Data::Initialize()
{
    data_.clear();
    reset();
}

// First use reset() to set the internal position to 0. Then you pass in the
// buffer where the results go. You pass in the length of that buffer. It
// returns how much was actually read. If you start at position 0, and read 100
// bytes, then you are now on position 100, and the next OTfread will proceed
// from that position. (Unless you reset().)
std::size_t Data::OTfread(std::uint8_t* data, const std::size_t& readSize)
{
    OT_ASSERT(data != nullptr && readSize > 0);

    std::size_t sizeToRead = 0;

    if (position_ < size()) {
        // If the size is 20, and position is 5 (I've already read the first 5
        // bytes) then the size remaining to read is 15. That is, GetSize()
        // minus position_.
        sizeToRead = size() - position_;

        if (readSize < sizeToRead) { sizeToRead = readSize; }

        OTPassword::safe_memcpy(data, readSize, &data_[position_], sizeToRead);
        position_ += sizeToRead;
    }

    return sizeToRead;
}

bool Data::Randomize(const std::size_t& size)
{
    SetSize(size);

    if (size == 0) { return false; }

    return OTPassword::randomizeMemory_uint8(data_.data(), size);
}

void Data::Release()
{
    zeroMemory();
    Initialize();
}

void Data::SetSize(const std::size_t& size)
{
    Release();

    if (size > 0) { data_.assign(size, 0); }
}

void Data::swap(opentxs::Data&& rhs)
{
    auto& in = dynamic_cast<Data&>(rhs);
    std::swap(data_, in.data_);
    std::swap(position_, in.position_);
}

void Data::zeroMemory()
{
    if (0 < data_.size()) { data_.assign(data_.size(), 0); }
}
}  // namespace implementation
}  // namespace opentxs
