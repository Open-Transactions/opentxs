// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/Sodium.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/crypto/library/OpenSSL.hpp"
#endif
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/Trezor.hpp"
#endif
#include "opentxs/OT.hpp"

#include "Hash.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Hash::"

namespace opentxs
{
api::crypto::Hash* Factory::Hash(
    api::crypto::Encode& encode,
    crypto::HashingProvider& ssl,
    crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
    ,
    crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& bitcoin
#endif
)
{
    return new api::crypto::implementation::Hash(
        encode,
        ssl,
        sodium
#if OT_CRYPTO_USING_TREZOR
        ,
        bitcoin
#endif
    );
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Hash::Hash(
    api::crypto::Encode& encode,
    opentxs::crypto::HashingProvider& ssl,
    opentxs::crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
    ,
    opentxs::crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& bitcoin
#endif
    )
    : encode_(encode)
    , ssl_(ssl)
    , sodium_(sodium)
#if OT_CRYPTO_USING_TREZOR
    , bitcoin_(bitcoin)
#endif
{
}

opentxs::crypto::HashingProvider& Hash::SHA2() const
{
#if OT_CRYPTO_SHA2_VIA_OPENSSL
    return ssl_;
#else
    return sodium_;
#endif
}

opentxs::crypto::HashingProvider& Hash::Sodium() const { return sodium_; }

bool Hash::Allocate(const proto::HashType hashType, OTPassword& input)
{
    return input.randomizeMemory(
        opentxs::crypto::HashingProvider::HashSize(hashType));
}

bool Hash::Allocate(const proto::HashType hashType, Data& input)
{
    return input.Randomize(
        opentxs::crypto::HashingProvider::HashSize(hashType));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256):
        case (proto::HASHTYPE_SHA512): {
            return SHA2().Digest(hashType, input, inputSize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return Sodium().Digest(hashType, input, inputSize, output);
        }
        case (proto::HASHTYPE_RIMEMD160): {
#if OT_CRYPTO_USING_TREZOR
            return bitcoin_.RIPEMD160(input, inputSize, output);
#endif
        }
        default: {
        }
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Unsupported hash type."
          << std::endl;

    return false;
}

bool Hash::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_SHA256):
        case (proto::HASHTYPE_SHA512): {
            return SHA2().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return Sodium().HMAC(
                hashType, input, inputSize, key, keySize, output);
        }
        default: {
        }
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Unsupported hash type."
          << std::endl;

    return false;
}

bool Hash::Digest(
    const proto::HashType hashType,
    const OTPassword& data,
    OTPassword& digest) const
{
    if (false == Allocate(hashType, digest)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to allocate output space." << std::endl;

        return false;
    }

    if (false == data.isMemory()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong OTPassword mode."
              << std::endl;

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.getMemory()),
        data.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const Data& data,
    Data& digest) const
{
    if (false == Allocate(hashType, digest)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to allocate output space." << std::endl;

        return false;
    }

    return Digest(
        hashType,
        static_cast<const std::uint8_t*>(data.data()),
        data.size(),
        static_cast<std::uint8_t*>(const_cast<void*>(digest.data())));
}

bool Hash::Digest(
    const proto::HashType hashType,
    const String& data,
    Data& digest) const
{
    if (false == Allocate(hashType, digest)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to allocate output space." << std::endl;

        return false;
    }

    return Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.Get()),
        data.GetLength(),
        static_cast<std::uint8_t*>(const_cast<void*>(digest.data())));
}

bool Hash::Digest(
    const std::uint32_t type,
    const std::string& data,
    std::string& encodedDigest) const
{
    proto::HashType hashType = static_cast<proto::HashType>(type);
    auto result = Data::Factory();

    if (false == Allocate(hashType, result)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to allocate output space." << std::endl;

        return false;
    }

    const bool success = Digest(
        hashType,
        reinterpret_cast<const std::uint8_t*>(data.c_str()),
        data.size(),
        static_cast<std::uint8_t*>(const_cast<void*>(result->data())));

    if (success) { encodedDigest.assign(encode_.IdentifierEncode(result)); }

    return success;
}

bool Hash::HMAC(
    const proto::HashType hashType,
    const OTPassword& key,
    const Data& data,
    OTPassword& digest) const
{
    if (false == Allocate(hashType, digest)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to allocate output space." << std::endl;

        return false;
    }

    if (false == key.isMemory()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong OTPassword mode."
              << std::endl;

        return false;
    }

    return HMAC(
        hashType,
        static_cast<const std::uint8_t*>(data.data()),
        data.size(),
        static_cast<const std::uint8_t*>(key.getMemory()),
        key.getMemorySize(),
        static_cast<std::uint8_t*>(digest.getMemoryWritable()));
}
}  // namespace opentxs::api::crypto::implementation
