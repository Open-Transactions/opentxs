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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP

#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <string>

namespace opentxs
{

class Data;
class OTPassword;
class String;

class CryptoUtil
{
protected:
    CryptoUtil() = default;
    virtual bool GetPasswordFromConsole(
        OTPassword& theOutput, const char* szPrompt) const = 0;

public:
    virtual bool RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const = 0;
    bool GetPasswordFromConsole(OTPassword& theOutput,
                                       bool bRepeat = false) const;

    virtual ~CryptoUtil() = default;
};
} // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOUTIL_HPP
