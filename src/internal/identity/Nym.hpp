// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/Nym.hpp"

#include "internal/identity/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace proto
{
class ContactData;
class HDPath;
class Nym;
class Signature;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Nym : virtual public identity::Nym
{
public:
    using Serialized = proto::Nym;

    enum class Mode : bool {
        Abbreviated = true,
        Full = false,
    };

    auto Internal() const noexcept -> const internal::Nym& final
    {
        return *this;
    }
    virtual auto Path(proto::HDPath& output) const -> bool = 0;
    using identity::Nym::PaymentCodePath;
    virtual auto PaymentCodePath(proto::HDPath& output) const -> bool = 0;
    using identity::Nym::Serialize;
    virtual auto Serialize(Serialized& serialized) const -> bool = 0;
    virtual auto SerializeCredentialIndex(
        AllocateOutput destination,
        const Mode mode) const -> bool = 0;
    virtual auto SerializeCredentialIndex(
        Serialized& serialized,
        const Mode mode) const -> bool = 0;
    virtual auto Sign(
        const google::protobuf::MessageLite& input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;
    virtual auto Verify(
        const google::protobuf::MessageLite& input,
        proto::Signature& signature) const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    auto Internal() noexcept -> internal::Nym& final { return *this; }
    virtual void SetAlias(const UnallocatedCString& alias) = 0;
    virtual void SetAliasStartup(const UnallocatedCString& alias) = 0;
    using identity::Nym::SetContactData;
    virtual auto SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool = 0;

    ~Nym() override = default;
};
}  // namespace opentxs::identity::internal
