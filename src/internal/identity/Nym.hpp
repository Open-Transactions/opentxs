// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/crypto/key/Keypair.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobufbuf
{
class MessageLite;
}  // namespace protobufbuf
}  // namespace google

namespace opentxs
{
namespace protobuf
{
class ContactData;
class HDPath;
class Nym;
class Signature;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Nym : virtual public identity::Nym
{
public:
    using Serialized = protobuf::Nym;

    enum class Mode : bool {
        Abbreviated = true,
        Full = false,
    };

    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    // 'S' (signing key) or
    // 'E' (encryption key) OR
    // 'A' (authentication key)
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    auto Internal() const noexcept -> const internal::Nym& final
    {
        return *this;
    }
    virtual auto Path(protobuf::HDPath& output) const -> bool = 0;
    using identity::Nym::PaymentCodePath;
    virtual auto PaymentCodePath(protobuf::HDPath& output) const -> bool = 0;
    using identity::Nym::Serialize;
    virtual auto Serialize(Serialized& serialized) const -> bool = 0;
    virtual auto SerializeCredentialIndex(Writer&& destination, const Mode mode)
        const -> bool = 0;
    virtual auto SerializeCredentialIndex(
        Serialized& serialized,
        const Mode mode) const -> bool = 0;
    virtual auto Sign(
        const google::protobuf::MessageLite& input,
        const crypto::SignatureRole role,
        protobuf::Signature& signature,
        const PasswordPrompt& reason,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;
    virtual auto Verify(
        const google::protobuf::MessageLite& input,
        protobuf::Signature& signature) const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    auto Internal() noexcept -> internal::Nym& final { return *this; }
    virtual void SetAlias(std::string_view alias) = 0;
    virtual void SetAliasStartup(std::string_view alias) = 0;
    using identity::Nym::SetContactData;
    virtual auto SetContactData(
        const protobuf::ContactData& data,
        const PasswordPrompt& reason) -> bool = 0;

    ~Nym() override = default;
};
}  // namespace opentxs::identity::internal
