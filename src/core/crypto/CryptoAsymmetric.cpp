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

#include "opentxs/core/crypto/CryptoAsymmetric.hpp"

#include "opentxs/core/crypto/OTSignature.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{

proto::AsymmetricKeyType CryptoAsymmetric::CurveToKeyType(
    const EcdsaCurve& curve)
{
    proto::AsymmetricKeyType output = proto::AKEYTYPE_ERROR;

    switch (curve) {
        case (EcdsaCurve::SECP256K1) : {
            output = proto::AKEYTYPE_SECP256K1;

            break;
        }
        case (EcdsaCurve::ED25519) : {
            output = proto::AKEYTYPE_ED25519;

            break;
        }
        default : {}
    }

    return output;
}

EcdsaCurve CryptoAsymmetric::KeyTypeToCurve(
    const proto::AsymmetricKeyType& type)
{
   EcdsaCurve output = EcdsaCurve::ERROR;

   switch (type) {
       case (proto::AKEYTYPE_SECP256K1) : {
           output = EcdsaCurve::SECP256K1;

           break;
       }
       case (proto::AKEYTYPE_ED25519) : {
           output = EcdsaCurve::ED25519;

           break;
       }
       default : {}
   }

   return output;
}

bool CryptoAsymmetric::SignContract(
    const String& strContractUnsigned,
    const OTAsymmetricKey& theKey,
    OTSignature& theSignature, // output
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    Data plaintext(strContractUnsigned.Get(), strContractUnsigned.GetLength()+1); //include null terminator
    Data signature;

    bool success = Sign(
                        plaintext,
                        theKey,
                        hashType,
                        signature,
                        pPWData);

    theSignature.SetData(signature, true); // true means, "yes, with newlines
                                              // in the b64-encoded output,
                                              // please."
    return success;
}

bool CryptoAsymmetric::VerifyContractSignature(
    const String& strContractToVerify,
    const OTAsymmetricKey& theKey,
    const OTSignature& theSignature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    Data plaintext(strContractToVerify.Get(), strContractToVerify.GetLength()+1); //include null terminator
    Data signature;
    theSignature.GetData(signature);

    return Verify(
            plaintext,
            theKey,
            signature,
            hashType,
            pPWData);

}

} // namespace opentxs
