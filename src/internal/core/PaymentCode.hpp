// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace credential
{
class Base;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Credential;
class PaymentCode;
class Signature;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::internal
{
class PaymentCode
{
public:
    using Serialized = proto::PaymentCode;

    virtual auto operator==(const Serialized& rhs) const noexcept -> bool = 0;

    virtual auto Serialize(Serialized& serialized) const noexcept -> bool = 0;
    virtual auto Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool = 0;

    virtual auto AddPrivateKeys(
        const crypto::SeedID& seed,
        const Bip32Index index,
        const opentxs::PasswordPrompt& reason) noexcept -> bool = 0;

    virtual ~PaymentCode() = default;
};
}  // namespace opentxs::internal
