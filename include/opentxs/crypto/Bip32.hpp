// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP32_HPP
#define OPENTXS_CRYPTO_BIP32_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs::crypto
{
std::string Print(const proto::HDPath& node);

template<typename X>
class Bip32
{
public:
    EXPORT virtual X AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const std::uint32_t index) const = 0;
    EXPORT virtual X GetChild(
        const proto::AsymmetricKey& parent,
        const std::uint32_t index) const = 0;
    EXPORT virtual X GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path) const = 0;
    EXPORT virtual X GetPaymentCode(
        std::string& fingerprint,
        const std::uint32_t nym) const = 0;
    EXPORT virtual X GetStorageKey(
        std::string& seed) const = 0;
    EXPORT virtual std::string Seed(
        const std::string& fingerprint = "") const = 0;
    EXPORT virtual std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
    EXPORT virtual X SeedToPrivateKey(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
};
}  // namespace opentxs::crypto
#endif  // OPENTXS_CRYPTO_BIP32_HPP
