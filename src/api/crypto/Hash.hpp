// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP
#define OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Hash : public api::crypto::Hash
{
public:
    bool Digest(
        const proto::HashType hashType,
        const OTPassword& data,
        OTPassword& digest) const override;
    bool Digest(const proto::HashType hashType, const Data& data, Data& digest)
        const override;
    bool Digest(
        const proto::HashType hashType,
        const String& data,
        Data& digest) const override;
    bool Digest(
        const std::uint32_t type,
        const std::string& data,
        std::string& encodedDigest) const override;
    bool HMAC(
        const proto::HashType hashType,
        const OTPassword& key,
        const Data& data,
        OTPassword& digest) const override;

    ~Hash() = default;

private:
    friend Factory;

    api::crypto::Encode& encode_;
    opentxs::crypto::HashingProvider& ssl_;
    opentxs::crypto::HashingProvider& sodium_;
#if OT_CRYPTO_USING_TREZOR
    opentxs::crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& bitcoin_;
#endif

    static bool Allocate(const proto::HashType hashType, OTPassword& input);
    static bool Allocate(const proto::HashType hashType, Data& input);

    bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const;
    Hash(
        api::crypto::Encode& encode,
        opentxs::crypto::HashingProvider& ssl,
        opentxs::crypto::HashingProvider& sodium
#if OT_CRYPTO_USING_TREZOR
        ,
        opentxs::crypto::Trezor<std::shared_ptr<proto::AsymmetricKey>>& bitcoin
#endif
    );
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const;
    opentxs::crypto::HashingProvider& SHA2() const;
    opentxs::crypto::HashingProvider& Sodium() const;

    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    Hash& operator=(const Hash&) = delete;
    Hash& operator=(Hash&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
#endif  // OPENTXS_CORE_CRYPTO_CRYPTOHASHENGINE_HPP
