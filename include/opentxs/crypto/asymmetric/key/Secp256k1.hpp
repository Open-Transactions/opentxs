// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class KeyPrivate;
}  // namespace asymmetric
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::asymmetric::key
{
class OPENTXS_EXPORT Secp256k1 : public HD
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Secp256k1&;

    auto UncompressedPubkey() const noexcept -> ReadView;

    OPENTXS_NO_EXPORT Secp256k1(KeyPrivate* imp) noexcept;
    Secp256k1(allocator_type alloc = {}) noexcept;
    Secp256k1(const Secp256k1& rhs, allocator_type alloc = {}) noexcept;
    Secp256k1(Secp256k1&& rhs) noexcept;
    Secp256k1(Secp256k1&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Secp256k1& rhs) noexcept -> Secp256k1&;
    auto operator=(Secp256k1&& rhs) noexcept -> Secp256k1&;

    ~Secp256k1() override;
};
}  // namespace opentxs::crypto::asymmetric::key
