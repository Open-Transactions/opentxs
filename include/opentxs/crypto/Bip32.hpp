// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <tuple>

#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace crypto
{
namespace key
{
class HD;
}  // namespace key

namespace internal
{
struct Bip32;
}  // namespace internal
}  // namespace crypto

namespace proto
{
class HDPath;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
auto Print(const proto::HDPath& node) noexcept -> UnallocatedCString;
auto Print(const proto::HDPath& node, bool showSeedID) noexcept
    -> UnallocatedCString;

class OPENTXS_EXPORT Bip32
{
public:
    struct Imp;

    using Path = UnallocatedVector<Bip32Index>;
    using Key = std::tuple<OTSecret, OTSecret, OTData, Path, Bip32Fingerprint>;

    auto DeriveKey(
        const EcdsaCurve& curve,
        const Secret& seed,
        const Path& path) const -> Key;
    /// throws std::runtime_error on invalid inputs
    auto DerivePrivateKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    /// throws std::runtime_error on invalid inputs
    auto DerivePublicKey(
        const key::HD& parent,
        const Path& pathAppend,
        const PasswordPrompt& reason) const noexcept(false) -> Key;
    auto DeserializePrivate(
        const UnallocatedCString& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Secret& key) const -> bool;
    auto DeserializePublic(
        const UnallocatedCString& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const -> bool;
    auto SeedID(const ReadView entropy) const -> OTIdentifier;
    auto SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Secret& key) const -> UnallocatedCString;
    auto SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const -> UnallocatedCString;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Bip32&;

    OPENTXS_NO_EXPORT Bip32(std::unique_ptr<Imp> imp) noexcept;
    OPENTXS_NO_EXPORT Bip32(Bip32&& rhs) noexcept;
    OPENTXS_NO_EXPORT ~Bip32();

private:
    std::unique_ptr<Imp> imp_;

    Bip32() = delete;
    Bip32(const Bip32&) = delete;
    auto operator=(const Bip32&) -> Bip32& = delete;
    auto operator=(Bip32&&) -> Bip32& = delete;
};
}  // namespace opentxs::crypto
