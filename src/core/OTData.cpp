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

#include <opentxs/core/OTData.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/util/Assert.hpp>

#include <cstdint>
#include <utility>
#include <cstring>

#include <thread>
#include <mutex>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Winsock2.h> // For htonl()
#endif

extern "C" {
#ifdef _WIN32
#else
#include <arpa/inet.h> // For htonl()
#endif
}

#ifndef _WIN32
#include <sys/mman.h> // for mlock and unlock
#endif

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
    else if ((static_cast<size_t>(-1) / _Size < _Count) ||
             (_Ptr = ::VirtualAlloc(NULL, _Count * _Size, MEM_COMMIT,
                                    PAGE_READWRITE)) == 0)
        std::bad_alloc(); // report no memory;
#else
    if (_Count == 0)
        ;
    else if ((static_cast<size_t>(-1) / _Size < _Count) ||
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
            otErr << __FUNCTION__
                  << " WARNING: %s: unable to lock memory, secret keys may "
                     "be swapped to disk!"; // UNIX
#else
        otErr << __FUNCTION__ << "ERROR: %s: no mlock support!";
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
            otErr << __FUNCTION__
                  << " WARNING: %s: unable to unlock memory, secret keys "
                     "may be swapped to disk!"; // UNIX
#else
        otErr << __FUNCTION__ << " ERROR: %s: no mlock support!";
        OT_FAIL; // OTHER (FAIL)
#endif
    }
}

namespace OTData
{

void appendShort(uint16_t in, ot_data_t& data)
{
    auto in_n = htons(in);
    auto in_n_p = reinterpret_cast<uint8_t*>(&in_n);
    data.insert(data.end(), in_n_p, in_n_p + sizeof(in_n));
}

void appendLong(uint32_t in, ot_data_t& data)
{
    auto in_n = htonl(in);
    auto in_n_p = reinterpret_cast<uint8_t*>(&in_n);
    data.insert(data.end(), in_n_p, in_n_p + sizeof(in_n));
}

uint16_t readShort(ot_data_t::const_iterator* it,
                   const ot_data_t::const_iterator& end)
{
    OT_ASSERT(nullptr != it);

    uint16_t out = 0;
    ot_data_t v(sizeof(out));

    for (auto& a : v) {
        if (*it == end) throw std::out_of_range("reached end of data");
        a = *(*it)++;
    }
    out = ntohs(*reinterpret_cast<uint16_t*>(v.data()));
    return out;
}

uint32_t readLong(ot_data_t::const_iterator* it,
                  const ot_data_t::const_iterator& end)
{
    OT_ASSERT(nullptr != it);

    uint32_t out = 0;
    ot_data_t v(sizeof(out));

    for (auto& a : v) {
        if (*it == end) throw std::out_of_range("reached end of data");
        a = *(*it)++;
    }
    out = ntohl(*reinterpret_cast<uint32_t*>(v.data()));
    return out;
}

void readDataVector(ot_data_t::const_iterator* it,
                    const ot_data_t::const_iterator& end, ot_data_t& out)
{
    OT_ASSERT(nullptr != it);

    for (auto& a : out) {
        if (*it == end) throw std::out_of_range("reached end of data");
        a = *(*it)++;
    }
}
}

} // namespace opentxs
