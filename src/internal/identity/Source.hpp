// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/String.hpp"
#include "opentxs/identity/Source.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Credential;
class NymIDSource;
class Signature;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Source : virtual public identity::Source
{
public:
    virtual auto asString() const noexcept -> OTString = 0;
    virtual auto Description() const noexcept -> OTString = 0;
    auto Internal() const noexcept -> const internal::Source& final
    {
        return *this;
    }

    auto Internal() noexcept -> internal::Source& final { return *this; }
    virtual auto Serialize(proto::NymIDSource& serialized) const noexcept
        -> bool = 0;
    virtual auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool = 0;
    virtual auto Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    Source(const Source&) = delete;
    Source(Source&&) = delete;
    auto operator=(const Source&) -> Source& = delete;
    auto operator=(Source&&) -> Source& = delete;

    ~Source() override = default;

protected:
    Source() = default;
};
}  // namespace opentxs::identity::internal
