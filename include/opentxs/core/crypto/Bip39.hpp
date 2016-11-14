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

#ifndef OPENTXS_CORE_CRYPTO_BIP39_HPP
#define OPENTXS_CORE_CRYPTO_BIP39_HPP

#include "opentxs/core/Proto.hpp"

#include <memory>
#include <string>

namespace opentxs
{
class OTPassword;

class Bip39
{
private:
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;

    bool DecryptSeed(
        const proto::Seed& seed,
        OTPassword& words,
        OTPassword& phrase) const;
    std::string SaveSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;
    bool SeedToData(
        const OTPassword& words,
        const OTPassword& passphrase,
        OTPassword& output) const;
    std::shared_ptr<proto::Seed> SerializedSeed(std::string& fingerprint) const;

    virtual bool toWords(
        const OTPassword& seed,
        OTPassword& words) const = 0;
    virtual void WordsToSeed(
        const OTPassword& words,
        OTPassword& seed,
        const OTPassword& passphrase) const = 0;

public:
    static const std::string DEFAULT_PASSPHRASE;

    std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;
    std::string NewSeed() const;
    std::string Passphrase(const std::string& fingerprint = "") const;
    std::shared_ptr<OTPassword> Seed(std::string& fingerprint) const;
    std::string Words(const std::string& fingerprint = "") const;
};
} // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_BIP39_HPP
