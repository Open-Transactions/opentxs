// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric

namespace symmetric
{
class Key;
}  // namespace symmetric

class Parameters;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
namespace credential
{
class Key;
}  // namespace credential

namespace internal
{
class Authority;
}  // namespace internal

class Source;
}  // namespace identity

class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
class OPENTXS_EXPORT Authority
{
public:
    virtual auto ContactCredentialVersion() const -> VersionNumber = 0;
    virtual auto GetMasterCredID() const -> identifier::Generic = 0;
    virtual auto GetTagCredential(crypto::asymmetric::Algorithm keytype) const
        noexcept(false) -> const credential::Key& = 0;
    virtual auto hasCapability(const NymCapability& capability) const
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Authority& = 0;
    virtual auto Params(const crypto::asymmetric::Algorithm type) const noexcept
        -> ReadView = 0;
    virtual auto Source() const -> const identity::Source& = 0;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Unlock(
        const crypto::asymmetric::Key& dhKey,
        const std::uint32_t tag,
        const crypto::asymmetric::Algorithm type,
        const crypto::symmetric::Key& key,
        PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto VerificationCredentialVersion() const -> VersionNumber = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Authority& = 0;

    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    auto operator=(const Authority&) -> Authority& = delete;
    auto operator=(Authority&&) -> Authority& = delete;

    virtual ~Authority() = default;

protected:
    Authority() noexcept = default;
};
}  // namespace opentxs::identity
