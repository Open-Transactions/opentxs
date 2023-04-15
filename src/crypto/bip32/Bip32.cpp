// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::Crypto

#include "opentxs/crypto/Bip32.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <memory>
#include <sstream>
#include <utility>

#include "crypto/HDNode.hpp"
#include "crypto/bip32/Imp.hpp"
#include "internal/crypto/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip32Child.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "util/HDIndex.hpp"

namespace opentxs::crypto
{
auto Print(const proto::HDPath& node) noexcept -> UnallocatedCString
{
    return Print(node, true);
}

auto Print(const proto::HDPath& node, bool showSeedID) noexcept
    -> UnallocatedCString
{
    auto output = std::stringstream{};

    if (showSeedID) {
        output << node.root();
    } else {
        output << 'm';
    }

    for (const auto& child : node.child()) {
        output << " / ";
        const Bip32Index max = HDIndex{Bip32Child::HARDENED};

        if (max > child) {
            output << std::to_string(child);
        } else {
            output << std::to_string(child - max) << "'";
        }
    }

    return output.str();
}
}  // namespace opentxs::crypto

namespace opentxs::factory
{
auto Bip32(const api::Crypto& crypto) noexcept -> crypto::Bip32
{
    return {std::make_unique<crypto::Bip32::Imp>(crypto)};
}
}  // namespace opentxs::factory

namespace opentxs::crypto
{
Bip32::Bip32(std::unique_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
}

Bip32::Bip32(Bip32&& rhs) noexcept
    : imp_()
{
    imp_.swap(rhs.imp_);
}

auto Bip32::DeriveKey(
    const EcdsaCurve& curve,
    const Secret& seed,
    const Path& path) const -> Key
{
    return imp_->DeriveKey(curve, seed, path);
}

auto Bip32::DerivePrivateKey(
    const asymmetric::key::HD& parent,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return imp_->DerivePrivateKey(parent, pathAppend, reason);
}

auto Bip32::DerivePublicKey(
    const asymmetric::key::HD& parent,
    const Path& pathAppend,
    const PasswordPrompt& reason) const noexcept(false) -> Key
{
    return imp_->DerivePublicKey(parent, pathAppend, reason);
}

auto Bip32::DeserializePrivate(
    std::string_view serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Secret& key) const -> bool
{
    return imp_->DeserializePrivate(
        serialized, network, depth, parent, index, chainCode, key);
}

auto Bip32::DeserializePublic(
    std::string_view serialized,
    Bip32Network& network,
    Bip32Depth& depth,
    Bip32Fingerprint& parent,
    Bip32Index& index,
    Data& chainCode,
    Data& key) const -> bool
{
    return imp_->DeserializePublic(
        serialized, network, depth, parent, index, chainCode, key);
}

auto Bip32::Internal() const noexcept -> const internal::Bip32&
{
    return *imp_;
}

auto Bip32::Internal() noexcept -> internal::Bip32& { return *imp_; }

auto Bip32::SeedID(const ReadView entropy) const -> identifier::Generic
{
    return imp_->SeedID(entropy);
}

auto Bip32::SerializePrivate(
    Bip32Network network,
    Bip32Depth depth,
    Bip32Fingerprint parent,
    Bip32Index index,
    ReadView chainCode,
    ReadView key,
    Writer&& out) const noexcept -> bool
{
    return imp_->SerializePrivate(
        network, depth, parent, index, chainCode, key, std::move(out));
}

auto Bip32::SerializePublic(
    Bip32Network network,
    Bip32Depth depth,
    Bip32Fingerprint parent,
    Bip32Index index,
    ReadView chainCode,
    ReadView key,
    Writer&& out) const noexcept -> bool
{
    return imp_->SerializePublic(
        network, depth, parent, index, chainCode, key, std::move(out));
}

Bip32::~Bip32() = default;
}  // namespace opentxs::crypto
