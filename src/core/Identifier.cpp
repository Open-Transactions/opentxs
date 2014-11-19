/************************************************************
 *
 *  Identifier.cpp
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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/Contract.hpp>
#include <opentxs/core/crypto/OTCachedKey.hpp>
#include <opentxs/core/crypto/OTCrypto.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/crypto/OTSymmetricKey.hpp>
#include <bitcoin-base58/hash.h>
#include <cstring>
#include <iostream>

namespace opentxs
{

Identifier::Identifier()
    : ot_data_t()
{
}

Identifier::Identifier(const Identifier& theID)
    : ot_data_t(theID)
{
}

Identifier::Identifier(const char* szStr)
    : ot_data_t()
{
    OT_ASSERT(nullptr != szStr);
    SetString(szStr);
}

Identifier::Identifier(const std::string& theStr)
    : ot_data_t()
{
    OT_ASSERT(!theStr.empty());
    SetString(theStr.c_str());
}

Identifier::Identifier(const String& theStr)
    : ot_data_t()
{
    SetString(theStr);
}

Identifier::Identifier(const Contract& theContract)
    : ot_data_t() // Get the contract's ID into this identifier.
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const OTPseudonym& theNym)
    : ot_data_t() // Get the Nym's ID into this identifier.
{
    (const_cast<OTPseudonym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const OTSymmetricKey& theKey)
    : ot_data_t() // Get the Symmetric Key's ID into *this. (It's a hash of the
                  // encrypted form of the symmetric key.)
{
    (const_cast<OTSymmetricKey&>(theKey)).GetIdentifier(*this);
}

Identifier::Identifier(const OTCachedKey& theKey)
    : ot_data_t() // Cached Key stores a symmetric key inside, so this actually
                  // captures the ID for that symmetrickey.
{
    const bool bSuccess =
        (const_cast<OTCachedKey&>(theKey)).GetIdentifier(*this);

    OT_ASSERT(bSuccess); // should never fail. If it does, then we are calling
                         // this function at a time we shouldn't, when we aren't
                         // sure the master key has even been generated yet. (If
                         // this asserts, need to examine the line of code that
                         // tried to do this, and figure out where its logic
                         // went wrong, since it should have made sure this
                         // would not happen, before constructing like this.)
}

void Identifier::SetString(const char* szString)
{
    OT_ASSERT(nullptr != szString);
    const String theStr(szString);
    SetString(theStr);
}

bool Identifier::operator==(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.Compare(ots2);
}

bool Identifier::operator!=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return !(ots1.Compare(ots2));
}

bool Identifier::operator>(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>(ots2);
}

bool Identifier::operator<(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<(ots2);
}

bool Identifier::operator<=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator<=(ots2);
}

bool Identifier::operator>=(const Identifier& s2) const
{
    const String ots1(*this), ots2(s2);
    return ots1.operator>=(ots2);
}

Identifier::~Identifier()
{
}

// When calling SignContract or VerifySignature with "HASH256" as the hash type,
// the signature will use (sha256 . sha256) as a message digest.
// In this case, SignContractDefaultHash and VerifyContractDefaultHash are used,
// which resort to low level calls to accomplish non standard message digests.
// Otherwise, it will use whatever OpenSSL provides by that name (see
// GetOpenSSLDigestByName).
const String Identifier::DefaultHashAlgorithm("HASH256");

// This method implements the (ripemd160 . sha256) hash,
// so the result is 20 bytes long.
bool Identifier::CalculateDigest(const uint8_t* data, size_t len)
{
    // The Hash160 function comes from the Bitcoin reference client, where
    // it is implemented as RIPEMD160 ( SHA256 ( x ) ) => 20 byte hash
    auto hash160 = Hash160(data, data + len);
    resize(20);
    assign(hash160, hash160 + 20);
    return true;
}

bool Identifier::CalculateDigest(const String& strInput)
{
    return CalculateDigest(
        reinterpret_cast<const unsigned char*>(strInput.Get()),
        static_cast<size_t>(strInput.GetLength()));
}

bool Identifier::CalculateDigest(const ot_data_t& dataInput)
{
    return CalculateDigest(dataInput.data(), dataInput.size());
}

// SET (binary id) FROM ENCODED STRING
//
void Identifier::SetString(const String& theStr)
{
    OTCrypto::It()->SetIDFromEncoded(theStr, *this);
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
//
void Identifier::GetString(String& theStr) const
{
    OTCrypto::It()->EncodeID(*this, theStr); // *this input, theStr output.
}

} // namespace opentxs
