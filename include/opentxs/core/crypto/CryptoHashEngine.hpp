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

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP

#include "opentxs/core/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{

class CryptoHash;
class CryptoEngine;
class Data;
class OTPassword;
class String;

// Singlton class for providing an interface to external crypto hashing
// libraries and hold the state required by those libraries.
class CryptoHashEngine
{
private:
    friend class CryptoEngine;

    CryptoHash& ssl_;
    CryptoHash& sodium_;

    CryptoHash& SHA2() const;
    CryptoHash& Sodium() const;

    static bool Allocate(const proto::HashType hashType, OTPassword& input);
    static bool Allocate(const proto::HashType hashType, Data& input);

    bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const;
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const;

    CryptoHashEngine(CryptoEngine& parent);
    CryptoHashEngine(const CryptoHashEngine&) = delete;
    CryptoHashEngine& operator=(const CryptoHashEngine&) = delete;

public:

    bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const;
    bool Digest(
        const proto::HashType hashType,
        const Data& data,
        Data& digest) const;
    bool Digest(
        const proto::HashType hashType,
        const String& data,
        Data& digest) const;
    bool Digest(
        const uint32_t type,
        const std::string& data,
        std::string& encodedDigest) const;

    bool HMAC(
        const proto::HashType hashType,
        const OTPassword& key,
        const Data& data,
        OTPassword& digest) const;

    ~CryptoHashEngine() = default;
};
}  // namespace opentxs
#endif // OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP
