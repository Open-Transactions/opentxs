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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOASYMMETRIC_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOASYMMETRIC_HPP

#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <set>

namespace opentxs
{

class OTAsymmetricKey;
class Data;
class OTPassword;
class OTPasswordData;
class OTSignature;

typedef std::multimap<std::string, OTAsymmetricKey*> mapOfAsymmetricKeys;

class CryptoAsymmetric
{

public:

    static proto::AsymmetricKeyType CurveToKeyType(const EcdsaCurve& curve);
    static EcdsaCurve KeyTypeToCurve(const proto::AsymmetricKeyType& type);

    bool SignContract(
        const String& strContractUnsigned,
        const OTAsymmetricKey& theKey,
        OTSignature& theSignature, // output
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;
    virtual bool VerifyContractSignature(
        const String& strContractToVerify,
        const OTAsymmetricKey& theKey,
        const OTSignature& theSignature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;

    virtual bool Sign(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const proto::HashType hashType,
        Data& signature, // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const = 0;
    virtual bool Verify(
        const Data& plaintext,
        const OTAsymmetricKey& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const = 0;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CRYPTOASYMMETRIC_HPP
