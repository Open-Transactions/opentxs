// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/verification/Nym.hpp"

#include "internal/identity/wot/verification/Item.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class Signature;
class VerificationGroup;
class VerificationIdentity;
class VerificationItem;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::internal
{
struct Nym : virtual public verification::Nym {
    using SerializedType = protobuf::VerificationIdentity;

    virtual operator SerializedType() const noexcept = 0;

    virtual auto API() const noexcept -> const api::Session& = 0;
    auto Internal() const noexcept -> const internal::Nym& final
    {
        return *this;
    }
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    using verification::Nym::AddItem;
    virtual auto AddItem(const Item::SerializedType item) noexcept -> bool = 0;
    auto Internal() noexcept -> internal::Nym& final { return *this; }
    virtual auto UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept -> bool = 0;

    ~Nym() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
