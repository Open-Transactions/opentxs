/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/Data.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"

#include <cstdio>
#include <iomanip>
#include <sstream>
#include <boost/algorithm/hex.hpp>

namespace opentxs
{
Data::Data(const OTASCIIArmor& source)
{
    if (source.Exists()) {
        source.GetData(*this);
    }
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

Data::Data(const Data& rhs)
    : position_(rhs.position_)
{
    Assign(rhs);
}

Data::Data(Data&& rhs)
{
    data_.swap(rhs.data_);
    position_ = rhs.position_;
    rhs.position_ = 0;
}

Data& Data::operator=(const Data& rhs)
{
    Assign(rhs);

    return *this;
}

Data& Data::operator=(Data&& rhs)
{
    swap(rhs);

    return *this;
}

bool Data::operator==(const Data& rhs) const { return data_ == rhs.data_; }

bool Data::operator!=(const Data& rhs) const { return !operator==(rhs); }

Data& Data::operator+=(const Data& rhs)
{
    concatenate(rhs.data_);

    return *this;
}

Data Data::fromHex(const std::string& in)
{
    std::vector<std::uint8_t> v;

    for (unsigned int i = 0; i < in.length(); i += 2) {
        std::string byteString = in.substr(i, 2);
        std::uint8_t byte =
            static_cast<std::uint8_t>(strtol(byteString.c_str(), NULL, 16));
        v.push_back(byte);
    }

    return Data(v);
}

std::string Data::asHex() const
{
    const std::size_t size = 2 * data_.size();
    std::vector<char> output{};
    output.resize(size, 0x0);

    for (std::size_t i = 0; i < data_.size(); i++) {
        std::sprintf(&output[2 * i], "%02X", data_.at(i));
    }

    return std::string(output.data(), output.size());
}

void Data::Assign(const Data& rhs)
{
    // can't assign to self.
    if (&rhs == this) {
        return;
    }

    data_ = rhs.data_;
    position_ = rhs.position_;
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
    for (const auto& byte : data) {
        data_.emplace_back(byte);
    }
}

void Data::Concatenate(const void* data, const std::size_t& size)
{
    OT_ASSERT(data != nullptr);
    OT_ASSERT(size > 0);

    if (size == 0) {
        return;
    }

    Data temp(data, size);
    concatenate(temp.data_);
}

bool Data::empty() const { return data_.empty(); }

const void* Data::GetPointer() const { return data_.data(); }

std::size_t Data::GetSize() const { return data_.size(); }

void Data::Initialize()
{
    data_.empty();
    reset();
}

bool Data::IsEmpty() const  // Deprecated.
{
    return empty();
}

// First use reset() to set the internal position to 0. Then you pass in the
// buffer where the results go. You pass in the length of that buffer. It
// returns how much was actually read. If you start at position 0, and read 100
// bytes, then you are now on position 100, and the next OTfread will proceed
// from that position. (Unless you reset().)
std::size_t Data::OTfread(std::uint8_t* data, const std::size_t& size)
{
    OT_ASSERT(data != nullptr && size > 0);

    std::size_t sizeToRead = 0;

    if (position_ < GetSize()) {
        // If the size is 20, and position is 5 (I've already read the first 5
        // bytes) then the size remaining to read is 15. That is, GetSize()
        // minus position_.
        sizeToRead = GetSize() - position_;

        if (size < sizeToRead) {
            sizeToRead = size;
        }

        OTPassword::safe_memcpy(data, size, &data_[position_], sizeToRead);
        position_ += sizeToRead;
    }

    return sizeToRead;
}

bool Data::Randomize(const std::size_t& size)
{
    SetSize(size);

    if (size == 0) {
        return false;
    }

    return OTPassword::randomizeMemory_uint8(data_.data(), size);
}

void Data::Release()
{
    zeroMemory();
    Initialize();
}

void Data::reset() { position_ = 0; }

void Data::SetSize(const std::size_t& size)
{
    Release();

    if (size > 0) {
        data_.assign(size, 0);
    }
}

void Data::swap(Data& rhs)
{
    std::swap(data_, rhs.data_);
    std::swap(position_, rhs.position_);
}

void Data::swap(Data&& rhs) { swap(rhs); }

void Data::zeroMemory()
{
    if (0 < data_.size()) {
        data_.assign(data_.size(), 0);
    }
}
}  // namespace opentxs
