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

#ifndef OPENTXS_CORE_CRYPTO_LETTER_HPP
#define OPENTXS_CORE_CRYPTO_LETTER_HPP

#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <list>
#include <map>
#include <string>

namespace opentxs

{
class AsymmetricKeyEC;
class Nym;
class OTPasswordData;
class Data;

typedef std::list<symmetricEnvelope> listOfSessionKeys;
typedef std::map<proto::AsymmetricKeyType, std::string> listOfEphemeralKeys;
typedef std::multimap<std::string, const AsymmetricKeyEC*> mapOfECKeys;

/** A letter is a contract that contains the contents of an OTEnvelope along
 *  with some necessary metadata.
 */
class Letter
{
private:
    static bool AddRSARecipients(
        const mapOfAsymmetricKeys& recipients,
        const SymmetricKey& sessionKey,
        proto::Envelope envelope);
    static bool DefaultPassword(OTPasswordData& password);
    static bool SortRecipients(
        const mapOfAsymmetricKeys& recipients,
        mapOfAsymmetricKeys& RSARecipients,
        mapOfECKeys& secp256k1Recipients,
        mapOfECKeys& ed25519Recipients);

    Letter() = default;

public:
    static bool Seal(
        const mapOfAsymmetricKeys& RecipPubKeys,
        const String& theInput,
        Data& dataOutput);
    static bool Open(
        const Data& dataInput,
        const Nym& theRecipient,
        const OTPasswordData& keyPassword,
        String& theOutput);

    ~Letter() = default;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_LETTER_HPP
