/************************************************************
 *
 *  OTData.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include "stdafx.hpp"

#include "OTData.hpp"
#include "crypto/OTASCIIArmor.hpp"
#include "crypto/OTPassword.hpp"
#include "util/Assert.hpp"
#include <utility>
#include <cstring>
#include <thread>
#include <mutex>

namespace opentxs
{

static std::mutex secure_allocator_mutex;

void* _SecureAllocateVoid(const size_t _Count, const size_t _Size)
{ // allocate storage for _Count elements of type _Ty

    std::lock_guard<std::mutex> lock(secure_allocator_mutex);

    void* _Ptr = 0;

#ifdef _WIN32
    if (_Count == 0)
        ;
    else if (((size_t)(-1) / _Size < _Count) ||
             (_Ptr = ::VirtualAlloc(NULL, _Count * _Size, MEM_COMMIT,
                                    PAGE_READWRITE)) == 0)
        std::bad_alloc(); // report no memory;
#else
    if (_Count == 0)
        ;
    else if (((size_t)(-1) / _Size < _Count) ||
             (_Ptr = ::operator new(_Count * _Size)) == 0)
        std::bad_alloc(); // report no memory
#endif

    if (NULL != _Ptr)
#ifdef _WIN32
        ::VirtualLock(_Ptr, _Count * _Size); // WINDOWS
#elif PREDEF_PLATFORM_UNIX
        // TODO:  Note: may need to add directives here so that mlock and
        // munlock are not
        // used except where the user is running as a privileged process.
        // (Because that
        // may be the only way we CAN use those functions...)
        if (mlock(_Ptr, _Count * _Size))
            OTLog::vError("WARNING: %s: unable to lock memory, secret keys may "
                          "be swapped to disk!",
                          __FUNCTION__); // UNIX
#else
        OTLog::vError("ERROR: %s: no mlock support!", __FUNCTION__);
    OT_FAIL; // OTHER (FAIL)
#endif

    return (_Ptr);
}

void _SecureDeallocateVoid(const size_t _Count, const size_t _Size, void* _Ptr)
{

    std::lock_guard<std::mutex> lock(secure_allocator_mutex); // must lock.

    if (NULL != _Ptr && 0 < _Count) {

#ifdef _WIN32
        ::SecureZeroMemory(_Ptr, _Count * _Size);
#else
        // This section securely overwrites the contents of a memory buffer
        // (which can otherwise be optimized out by an overzealous compiler...)
        volatile uint8_t* _vPtr = static_cast<volatile uint8_t*>(_Ptr);
        {
            size_t count = _Count * (_Size / sizeof(uint8_t));
            while (count--) *_vPtr++ = 0;
        }
#endif

#ifdef _WIN32
        ::VirtualUnlock(_Ptr, _Count * _Size);
#elif PREDEF_PLATFORM_UNIX
        if (munlock(_Ptr, _Count * _Size))
            OTLog::vError("WARNING: %s: unable to unlock memory, secret keys "
                          "may be swapped to disk!",
                          __FUNCTION__); // UNIX
#else
        OTLog::vError("ERROR: %s: no mlock support!", __FUNCTION__);
        OT_FAIL; // OTHER (FAIL)
#endif
    }
};

OTData::OTData()
    : data_(nullptr)
    , position_(0)
    , size_(0)
{
}

OTData::OTData(const OTData& source)
    : data_(nullptr)
    , position_(0)
    , size_(0)
{
    Assign(source);
}

OTData::OTData(const OTASCIIArmor& source)
    : data_(nullptr)
    , position_(0)
    , size_(0)
{
    if (source.Exists()) {
        source.GetData(*this);
    }
}

OTData::OTData(const void* data, uint32_t size)
    : data_(nullptr)
    , position_(0)
    , size_(0)
{
    Assign(data, size);
}

OTData::~OTData()
{
    Release();
}

bool OTData::operator==(const OTData& rhs) const
{
    if (size_ != rhs.size_) {
        return false;
    }

    if (size_ == 0 && rhs.size_ == 0) {
        return true;
    }
    // TODO security: replace memcmp with a more secure
    // version. Still, though, I am managing it internal to
    // the class.
    if (std::memcmp(data_, rhs.data_, size_) == 0) {
        return true;
    }

    return false;
}

bool OTData::operator!=(const OTData& rhs) const
{
    return !operator==(rhs);
}

// First use reset() to set the internal position to 0.
// Then you pass in the buffer where the results go.
// You pass in the length of that buffer.
// It returns how much was actually read.
// If you start at position 0, and read 100 bytes, then
// you are now on position 100, and the next OTfread will
// proceed from that position. (Unless you reset().)
uint32_t OTData::OTfread(uint8_t* data, uint32_t size)
{
    OT_ASSERT(data != nullptr && size > 0);

    uint32_t sizeToRead = 0;

    if (data_ != nullptr && position_ < GetSize()) {
        // If the size is 20, and position is 5 (I've already read the first 5
        // bytes) then the size remaining to read is 15. That is, GetSize()
        // minus position_.
        sizeToRead = GetSize() - position_;

        if (size < sizeToRead) {
            sizeToRead = size;
        }
        OTPassword::safe_memcpy(data, size,
                                static_cast<uint8_t*>(data_) + position_,
                                static_cast<uint32_t>(sizeToRead));
        position_ += sizeToRead;
    }

    return sizeToRead;
}

void OTData::zeroMemory() const
{
    if (data_ != nullptr) {
        OTPassword::zeroMemory(data_, size_);
    }
}

void OTData::Release()
{
    if (data_ != nullptr) {
        // For security reasons, we clear the memory to 0 when deleting the
        // object. (Seems smart.)
        OTPassword::zeroMemory(data_, size_);
        delete[] static_cast<uint8_t*>(data_);
        // If data_ was already nullptr, no need to re-Initialize().
        Initialize();
    }
}

OTData& OTData::operator=(OTData rhs)
{
    swap(rhs);
    return *this;
}

void OTData::swap(OTData& rhs)
{
    std::swap(data_, rhs.data_);
    std::swap(position_, rhs.position_);
    std::swap(size_, rhs.size_);
}

void OTData::Assign(const OTData& source)
{
    // can't assign to self.
    if (&source == this) {
        return;
    }

    if (!source.IsEmpty()) {
        Assign(source.data_, source.size_);
    }
    else {
        // Otherwise if it's empty, then empty this also.
        Release();
    }
}

bool OTData::IsEmpty() const
{
    return size_ < 1;
}

void OTData::Assign(const void* data, uint32_t size)
{
    // This releases all memory and zeros out all members.
    Release();

    if (data != nullptr && size > 0) {
        data_ = static_cast<void*>(new uint8_t[size]);
        OT_ASSERT(data_ != nullptr);
        OTPassword::safe_memcpy(data_, size, data, size);
        size_ = size;
    }
    // TODO: else error condition.  Could just ASSERT() this.
}

bool OTData::Randomize(uint32_t size)
{
    Release(); // This releases all memory and zeros out all members.
    if (size > 0) {
        data_ = static_cast<void*>(new uint8_t[size]);
        OT_ASSERT(data_ != nullptr);

        if (!OTPassword::randomizeMemory_uint8(static_cast<uint8_t*>(data_),
                                               size)) {
            // randomizeMemory already logs, so I'm not logging again twice
            // here.
            delete[] static_cast<uint8_t*>(data_);
            data_ = nullptr;
            return false;
        }

        size_ = size;
        return true;
    }
    // else error condition.  Could just ASSERT() this.
    return false;
}

void OTData::Concatenate(const void* data, uint32_t size)
{
    OT_ASSERT(data != nullptr);
    OT_ASSERT(size > 0);

    if (size == 0) {
        return;
    }

    if (size_ == 0) {
        Assign(data, size);
        return;
    }

    void* newData = nullptr;
    uint32_t newSize = GetSize() + size;

    if (newSize > 0) {
        newData = static_cast<void*>(new uint8_t[newSize]);
        OT_ASSERT(newData != nullptr);
        OTPassword::zeroMemory(newData, newSize);
    }
    // If there's a new memory buffer (for the combined..)
    if (newData != nullptr) {
        // if THIS object has data inside of it...
        if (!IsEmpty()) {
            // Copy THIS object into the new
            // buffer, starting at the
            // beginning.
            OTPassword::safe_memcpy(newData, newSize, data_, GetSize());
        }

        // Next we copy the data being appended...
        OTPassword::safe_memcpy(static_cast<uint8_t*>(newData) + GetSize(),
                                newSize - GetSize(), data, size);
    }

    if (data_ != nullptr) {
        delete[] static_cast<uint8_t*>(data_);
    }

    data_ = newData;
    size_ = newSize;
}

OTData& OTData::operator+=(const OTData& rhs)
{
    if (rhs.GetSize() > 0) {
        Concatenate(rhs.data_, rhs.GetSize());
    }
    return *this;
}

void OTData::SetSize(uint32_t size)
{
    Release();

    if (size > 0) {
        data_ = static_cast<void*>(new uint8_t[size]);
        OT_ASSERT(data_ != nullptr);
        OTPassword::zeroMemory(data_, size);
        size_ = size;
    }
}

} // namespace opentxs
