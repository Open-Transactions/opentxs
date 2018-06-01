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

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_WITH_BIP39

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{

class OTPassword;

namespace api
{
class Native;
}  // namespace api

class Bip39
{
public:
    static const std::string DEFAULT_PASSPHRASE;

    std::string DefaultSeed() const;
    std::string ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;
    std::string NewSeed() const;
    std::string Passphrase(const std::string& fingerprint = "") const;
    std::shared_ptr<OTPassword> Seed(
        std::string& fingerprint,
        std::int32_t& index) const;
    bool UpdateIndex(std::string& seed, const std::int32_t index) const;
    std::string Words(const std::string& fingerprint = "") const;

    virtual ~Bip39() = default;

protected:
    Bip39(api::Native& native);

private:
    api::Native& native_;

    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;

    bool DecryptSeed(
        const proto::Seed& seed,
        OTPassword& words,
        OTPassword& phrase) const;
    std::string SaveSeed(const OTPassword& words, const OTPassword& passphrase)
        const;
    bool SeedToData(
        const OTPassword& words,
        const OTPassword& passphrase,
        OTPassword& output) const;
    std::shared_ptr<proto::Seed> SerializedSeed(
        std::string& fingerprint,
        std::int32_t& index) const;

    virtual bool toWords(const OTPassword& seed, OTPassword& words) const = 0;
    virtual void WordsToSeed(
        const OTPassword& words,
        OTPassword& seed,
        const OTPassword& passphrase) const = 0;

    Bip39() = delete;
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    Bip39& operator=(const Bip39&) = delete;
    Bip39& operator=(Bip39&&) = delete;
};
}  // namespace opentxs

#endif  // OT_CRYPTO_WITH_BIP39
#endif  // OPENTXS_CORE_CRYPTO_BIP39_HPP
