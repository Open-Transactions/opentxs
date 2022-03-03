// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Nym.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
struct Authority : virtual public identity::Authority {
    static auto NymToContactCredential(const VersionNumber nym) noexcept(false)
        -> VersionNumber;

    virtual auto GetMasterCredential() const -> const credential::Primary& = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    ~Authority() override = default;
};
struct Nym : virtual public identity::Nym {
    enum class Mode : bool {
        Abbreviated = true,
        Full = false,
    };

    virtual auto SerializeCredentialIndex(
        AllocateOutput destination,
        const Mode mode) const -> bool = 0;
    virtual auto SerializeCredentialIndex(
        Serialized& serialized,
        const Mode mode) const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    virtual void SetAlias(const UnallocatedCString& alias) = 0;
    virtual void SetAliasStartup(const UnallocatedCString& alias) = 0;

    ~Nym() override = default;
};
}  // namespace opentxs::identity::internal
