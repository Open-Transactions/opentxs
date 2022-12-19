// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/hd/HDPrivate.hpp"  // IWYU pragma: associated

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
auto HD::Blank() noexcept -> HD&
{
    static auto blank = HD{};

    return blank;
}

auto HD::CalculateFingerprint(
    const api::crypto::Hash& hash,
    const ReadView key) noexcept -> Bip32Fingerprint
{
    auto output = Bip32Fingerprint{0};
    auto digest = ByteArray{};

    if (33 != key.size()) {
        LogError()(OT_PRETTY_STATIC(HD))("Invalid public key").Flush();

        return output;
    }

    const auto hashed =
        hash.Digest(crypto::HashType::Bitcoin, key, digest.WriteInto());

    if (false == hashed) {
        LogError()(OT_PRETTY_STATIC(HD))("Failed to calculate public key hash")
            .Flush();

        return output;
    }

    if (false == digest.Extract(output)) {
        LogError()(OT_PRETTY_STATIC(HD))("Failed to set output").Flush();

        return {};
    }

    return output;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
HDPrivate::HDPrivate(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
{
}

HDPrivate::HDPrivate(const HDPrivate& rhs, allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(rhs, alloc)
{
}

auto HDPrivate::Chaincode(const PasswordPrompt&) const noexcept -> ReadView
{
    return {};
}

auto HDPrivate::ChildKey(
    const Bip32Index,
    const PasswordPrompt&,
    allocator_type) const noexcept -> asymmetric::key::HD
{
    return {};
}

auto HDPrivate::Depth() const noexcept -> int { return -1; }

auto HDPrivate::Fingerprint() const noexcept -> Bip32Fingerprint { return {}; }

auto HDPrivate::Parent() const noexcept -> Bip32Fingerprint { return {}; }

auto HDPrivate::Xprv(const PasswordPrompt&, Writer&&) const noexcept -> bool
{
    return {};
}

auto HDPrivate::Xpub(const PasswordPrompt&, Writer&&) const noexcept -> bool
{
    return {};
}

HDPrivate::~HDPrivate() = default;
}  // namespace opentxs::crypto::asymmetric::key
