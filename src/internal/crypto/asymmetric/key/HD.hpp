// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Hash;
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::internal::key
{
class HD : virtual public EllipticCurve
{
public:
    static auto Blank() noexcept -> HD&;
    static auto CalculateFingerprint(
        const api::crypto::Hash& hash,
        const ReadView pubkey) noexcept -> Bip32Fingerprint;

    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD& rhs) noexcept -> HD& = delete;
    auto operator=(HD&& rhs) noexcept -> HD& = delete;

    ~HD() override = default;

protected:
    HD() = default;
};
}  // namespace opentxs::crypto::asymmetric::internal::key
