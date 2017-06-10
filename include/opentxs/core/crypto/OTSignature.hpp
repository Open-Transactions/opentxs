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

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"

namespace opentxs
{

class String;

class OTSignature : public OTASCIIArmor
{
public:
    OTSignature()
        : OTASCIIArmor()
    {
    }

    virtual ~OTSignature() {}

    explicit OTSignature(const String& value)
        : OTASCIIArmor(value)
    {
    }

    explicit OTSignature(const OTASCIIArmor& value)
        : OTASCIIArmor(value)
    {
    }

    explicit OTSignature(const char* value)
        : OTASCIIArmor(value)
    {
    }

    OTSignatureMetadata& getMetaData() { return metadata_; }

    const OTSignatureMetadata& getMetaData() const { return metadata_; }

private:
    OTSignatureMetadata metadata_;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTSIGNATURE_HPP
