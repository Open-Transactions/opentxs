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

#include "opentxs/core/OTData.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"

namespace opentxs
{
OTData::OTData(const OTASCIIArmor& source)
{
    if (source.Exists()) {
        source.GetData(*this);
    }
}

OTData::OTData(const void* data, std::size_t size)
    : data_(
          static_cast<const std::uint8_t*>(data),
          static_cast<const std::uint8_t*>(data) + size)
{
}

OTData::OTData(const std::vector<unsigned char>& sourceVector)
{
    Assign(sourceVector.data(), sourceVector.size());
}

OTData::OTData(const OTData& rhs)
    : position_(rhs.position_)
{
    Assign(rhs);
}

OTData::OTData(OTData&& rhs)
{
    data_.swap(rhs.data_);
    position_ = rhs.position_;
    rhs.position_ = 0;
}

OTData& OTData::operator=(const OTData& rhs)
{
    Assign(rhs);

    return *this;
}

OTData& OTData::operator=(OTData&& rhs)
{
    swap(rhs);

    return *this;
}

bool OTData::operator==(const OTData& rhs) const { return data_ == rhs.data_; }

bool OTData::operator!=(const OTData& rhs) const { return !operator==(rhs); }

OTData& OTData::operator+=(const OTData& rhs)
{
    concatenate(rhs.data_);

    return *this;
}

void OTData::Assign(const OTData& rhs)
{
    // can't assign to self.
    if (&rhs == this) {
        return;
    }

    data_ = rhs.data_;
    position_ = rhs.position_;
}

void OTData::Assign(const void* data, const std::size_t& size)
{
    Release();

    if (data != nullptr && size > 0) {
        auto start = static_cast<const std::uint8_t*>(data);
        const std::uint8_t* end = start + size;
        data_.assign(start, end);
    }
}

void OTData::concatenate(const Vector& data)
{
    for (const auto& byte : data) {
        data_.emplace_back(byte);
    }
}

void OTData::Concatenate(const void* data, const std::size_t& size)
{
    OT_ASSERT(data != nullptr);
    OT_ASSERT(size > 0);

    if (size == 0) {
        return;
    }

    Data temp(data, size);
    concatenate(temp.data_);
}

bool OTData::empty() const { return data_.empty(); }

const void* OTData::GetPointer() const { return data_.data(); }

std::size_t OTData::GetSize() const { return data_.size(); }

void OTData::Initialize()
{
    data_.empty();
    reset();
}

bool OTData::IsEmpty() const  // Deprecated.
{
    return empty();
}

// First use reset() to set the internal position to 0. Then you pass in the
// buffer where the results go. You pass in the length of that buffer. It
// returns how much was actually read. If you start at position 0, and read 100
// bytes, then you are now on position 100, and the next OTfread will proceed
// from that position. (Unless you reset().)
std::size_t OTData::OTfread(std::uint8_t* data, const std::size_t& size)
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

bool OTData::Randomize(const std::size_t& size)
{
    SetSize(size);

    if (size == 0) {
        return false;
    }

    return OTPassword::randomizeMemory_uint8(data_.data(), size);
}

void OTData::Release()
{
    zeroMemory();
    Initialize();
}

void OTData::reset() { position_ = 0; }

void OTData::SetSize(const std::size_t& size)
{
    Release();

    if (size > 0) {
        data_.assign(size, 0);
    }
}

void OTData::swap(OTData& rhs)
{
    std::swap(data_, rhs.data_);
    std::swap(position_, rhs.position_);
}

void OTData::swap(OTData&& rhs) { swap(rhs); }

void OTData::zeroMemory()
{
    if (0 < data_.size()) {
        data_.assign(data_.size(), 0);
    }
}
}  // namespace opentxs
