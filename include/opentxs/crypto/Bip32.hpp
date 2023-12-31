// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace key
{
class HD;
}  // namespace key
}  // namespace asymmetric

namespace internal
{
struct Bip32;
}  // namespace internal
}  // namespace crypto

namespace protobuf
{
class HDPath;
}  // namespace protobuf

class ByteArray;
class Data;
class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
auto Print(const protobuf::HDPath& node) noexcept -> UnallocatedCString;
auto Print(const protobuf::HDPath& node, bool showSeedID) noexcept
    -> UnallocatedCString;

class OPENTXS_EXPORT Bip32
{
public:
    struct Imp;

    using Path = UnallocatedVector<Bip32Index>;
    using Key = std::tuple<Secret, Secret, ByteArray, Path, Bip32Fingerprint>;

    auto DeriveKey(
        const EcdsaCurve& curve,
        const Secret& seed,
        const Path& path) const -> Key;
    /// throws std::runtime_error on invalid inputs
    auto DerivePrivateKey(
        const asymmetric::key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    /// throws std::runtime_error on invalid inputs
    auto DerivePublicKey(
        const asymmetric::key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DeserializePrivate(
        std::string_view serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const -> bool;
    auto DeserializePublic(
        std::string_view serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const -> bool;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Bip32&;
    auto SeedID(const ReadView entropy) const noexcept -> crypto::SeedID;
    auto SerializePrivate(
        Bip32Network network,
        Bip32Depth depth,
        Bip32Fingerprint parent,
        Bip32Index index,
        ReadView chainCode,
        ReadView key,
        Writer&& out) const noexcept -> bool;
    auto SerializePublic(
        Bip32Network network,
        Bip32Depth depth,
        Bip32Fingerprint parent,
        Bip32Index index,
        ReadView chainCode,
        ReadView key,
        Writer&& out) const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Bip32&;

    OPENTXS_NO_EXPORT Bip32(std::unique_ptr<Imp> imp) noexcept;
    OPENTXS_NO_EXPORT Bip32(Bip32&& rhs) noexcept;
    Bip32() = delete;
    Bip32(const Bip32&) = delete;
    auto operator=(const Bip32&) -> Bip32& = delete;
    auto operator=(Bip32&&) -> Bip32& = delete;

    OPENTXS_NO_EXPORT ~Bip32();

private:
    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::crypto
