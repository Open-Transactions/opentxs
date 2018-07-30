// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_BIP32_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_BIP32_HPP

#include "Internal.hpp"

#include "opentxs/crypto/Bip32.hpp"

namespace opentxs::crypto::implementation
{
    
typedef std::shared_ptr<proto::AsymmetricKey> TrezorKey;

class Bip32 : virtual public opentxs::crypto::Bip32<TrezorKey>
{
public:
    TrezorKey AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const std::uint32_t index) const;
    std::string Seed(const std::string& fingerprint = "") const;
    TrezorKey GetPaymentCode(
        std::string& fingerprint,
        const std::uint32_t nym) const;
    TrezorKey GetStorageKey(
        std::string& seed) const;
};
}  // namespace opentxs::crypto::implementation
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_BIP32_HPP
