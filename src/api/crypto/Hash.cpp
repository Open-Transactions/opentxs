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
#if OT_CRYPTO_USING_LIBBITCOIN
#include "opentxs/crypto/library/Bitcoin.hpp"
#endif
#include "opentxs/crypto/library/EncodingProvider.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/Ripemd160.hpp"
#include "opentxs/crypto/library/Sodium.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/Trezor.hpp"
#endif
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/crypto/library/OpenSSL.hpp"
#endif
#include "opentxs/OT.hpp"

#include "Hash.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Hash::"

namespace opentxs
{
api::crypto::Hash* Factory::Hash(
    const api::crypto::Encode& encode,
    const crypto::HashingProvider& ssl,
    const crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
    ,
    const crypto::Ripemd160& bitcoin
#endif
)
{
    return new api::crypto::implementation::Hash(
        encode,
        ssl,
        sodium
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
        ,
        bitcoin
#endif
    );
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Hash::Hash(
    const api::crypto::Encode& encode,
    const opentxs::crypto::HashingProvider& ssl,
    const opentxs::crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
    ,
    const opentxs::crypto::Ripemd160& bitcoin
#endif
    )
    : encode_(encode)
    , ssl_(ssl)
    , sodium_(sodium)
#if OT_CRYPTO_USING_TREZOR || OT_CRYPTO_USING_LIBBITCOIN
    , bitcoin_(bitcoin)
#endif
{
}

const opentxs::crypto::HashingProvider& Hash::SHA2() const
{
#if OT_CRYPTO_SHA2_VIA_OPENSSL
    return ssl_;
#else
    return sodium_;
#endif
}

const opentxs::crypto::HashingProvider& Hash::Sodium() const { return sodium_; }

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
#if OT_CRYPTO_USING_LIBBITCOIN
            return dynamic_cast<const opentxs::crypto::Bitcoin&>(bitcoin_)
                .RIPEMD160(input, inputSize, output);
#elif OT_CRYPTO_USING_TREZOR
            return dynamic_cast<const opentxs::crypto::Trezor&>(bitcoin_)
                .RIPEMD160(input, inputSize, output);
#endif
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash type.").Flush();

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

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash type.").Flush();

    return false;
}

bool Hash::Digest(
    const proto::HashType hashType,
    const OTPassword& data,
    OTPassword& digest) const
{
    if (false == Allocate(hashType, digest)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    if (false == data.isMemory()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong OTPassword mode.").Flush();

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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to allocate output space.")
            .Flush();

        return false;
    }

    if (false == key.isMemory()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong OTPassword mode.").Flush();

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
