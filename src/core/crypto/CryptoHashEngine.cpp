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

#include "opentxs/core/crypto/CryptoHashEngine.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/Libsodium.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/core/crypto/OpenSSL.hpp"
#endif
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
CryptoHashEngine::CryptoHashEngine(CryptoEngine& parent)
    : ssl_(*parent.ssl_)
    , sodium_(*parent.ed25519_)
{
}

CryptoHash& CryptoHashEngine::SHA2() const
{
#if OT_CRYPTO_SHA2_VIA_OPENSSL
    return ssl_;
#else
    return sodium_;
#endif
}

CryptoHash& CryptoHashEngine::Sodium() const
{
    return sodium_;
}

bool CryptoHashEngine::Allocate(
    const proto::HashType hashType,
    OTPassword& input)
{
    return input.randomizeMemory(CryptoHash::HashSize(hashType));
}

bool CryptoHashEngine::Allocate(
    const proto::HashType hashType,
    Data& input)
{
    return input.Randomize(CryptoHash::HashSize(hashType));
}

bool CryptoHashEngine::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256) :
        case (proto::HASHTYPE_SHA512) : {
            return SHA2().Digest(hashType, input, inputSize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160) :
        case (proto::HASHTYPE_BLAKE2B256) :
        case (proto::HASHTYPE_BLAKE2B512) : {
            return Sodium().Digest(hashType, input, inputSize, output);
        }
        default : {}
    }

    otErr << __FUNCTION__ << ": Unsupported hash type." << std::endl;

    return false;
}

bool CryptoHashEngine::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256) :
        case (proto::HASHTYPE_SHA512) : {
            return SHA2().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160) :
        case (proto::HASHTYPE_BLAKE2B256) :
        case (proto::HASHTYPE_BLAKE2B512) : {
            return Sodium().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        default : {}
    }

    otErr << __FUNCTION__ << ": Unsupported hash type." << std::endl;

    return false;
}

bool CryptoHashEngine::Digest(
    const proto::HashType hashType,
    const OTPassword& data,
    OTPassword& digest) const
{
    if (!Allocate(hashType, digest)) { return false; }

    if (!data.isMemory()) { return false; }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.getMemory()),
        data.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}

bool CryptoHashEngine::Digest(
    const proto::HashType hashType,
    const Data& data,
    Data& digest) const
{
    if (!Allocate(hashType, digest)) { return false; }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.GetPointer()),
        data.GetSize(),
        static_cast<std::uint8_t*>(const_cast<void*>(digest.GetPointer())));
}

bool CryptoHashEngine::Digest(
    const proto::HashType hashType,
    const String& data,
    Data& digest) const
{
    if (!Allocate(hashType, digest)) { return false; }

    return Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.Get()),
        data.GetLength(),
        static_cast<std::uint8_t*>(const_cast<void*>(digest.GetPointer())));
}

bool CryptoHashEngine::Digest(
    const uint32_t type,
    const std::string& data,
    std::string& encodedDigest) const
{
    proto::HashType hashType = static_cast<proto::HashType>(type);
    Data result;

    if (!Allocate(hashType, result)) { return false; }

    const bool success = Digest(
        hashType,
        reinterpret_cast<const uint8_t*>(data.c_str()),
        data.size(),
        static_cast<std::uint8_t*>(const_cast<void*>(result.GetPointer())));

    if (success) {
        encodedDigest.assign(
            OT::App().Crypto().Encode().IdentifierEncode(result));
    }

    return success;
}

bool CryptoHashEngine::HMAC(
    const proto::HashType hashType,
    const OTPassword& key,
    const Data& data,
    OTPassword& digest) const
{
    if (!key.isMemory()) { return false; }

    if (!Allocate(hashType, digest)) { return false; }

    return HMAC(
        hashType,
        static_cast<const std::uint8_t*>(data.GetPointer()),
        data.GetSize(),
        static_cast<const std::uint8_t*>(key.getMemory()),
        key.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}
} // namespace opentxs
