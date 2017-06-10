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

#include "opentxs/core/crypto/CryptoHash.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <string>

namespace opentxs
{
proto::HashType CryptoHash::StringToHashType(const String& inputString)
{
    if (inputString.Compare("NULL"))
        return proto::HASHTYPE_NONE;
    else if (inputString.Compare("SHA256"))
        return proto::HASHTYPE_SHA256;
    else if (inputString.Compare("SHA512"))
        return proto::HASHTYPE_SHA512;
    else if (inputString.Compare("BLAKE2B160"))
        return proto::HASHTYPE_BLAKE2B160;
    else if (inputString.Compare("BLAKE2B256"))
        return proto::HASHTYPE_BLAKE2B256;
    else if (inputString.Compare("BLAKE2B512"))
        return proto::HASHTYPE_BLAKE2B512;
    return proto::HASHTYPE_ERROR;
}

String CryptoHash::HashTypeToString(const proto::HashType hashType)

{
    String hashTypeString;

    switch (hashType) {
        case proto::HASHTYPE_NONE :
            hashTypeString = "NULL";
            break;
        case proto::HASHTYPE_SHA256 :
            hashTypeString = "SHA256";
            break;
        case proto::HASHTYPE_SHA512 :
            hashTypeString = "SHA512";
            break;
        case proto::HASHTYPE_BLAKE2B160 :
            hashTypeString = "BLAKE2B160";
            break;
        case proto::HASHTYPE_BLAKE2B256 :
            hashTypeString = "BLAKE2B256";
            break;
        case proto::HASHTYPE_BLAKE2B512 :
            hashTypeString = "BLAKE2B512";
            break;
        default :
            hashTypeString = "ERROR";
    }
    return hashTypeString;
}

size_t CryptoHash::HashSize(const proto::HashType hashType)
{
    switch (hashType) {
        case proto::HASHTYPE_SHA256 : { return 32; }
        case proto::HASHTYPE_SHA512 : { return 64; }
        case proto::HASHTYPE_BLAKE2B160 : { return 20; }
        case proto::HASHTYPE_BLAKE2B256 : { return 32; }
        case proto::HASHTYPE_BLAKE2B512 : { return 64; }
        default : {}
    }

    return 0;
}
} // namespace opentxs
