// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/verification/Group.hpp"

#include "internal/identity/wot/verification/Item.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Signature;
class Verification;
class VerificationGroup;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::internal
{
struct Group : virtual public verification::Group {
    using SerializedType = proto::VerificationGroup;

    virtual operator SerializedType() const noexcept = 0;

    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto External() const noexcept -> bool = 0;
    auto Internal() const noexcept -> const internal::Group& final
    {
        return *this;
    }
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    using verification::Group::AddItem;
    virtual auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool = 0;
    auto Internal() noexcept -> internal::Group& final { return *this; }
    virtual void Register(
        const identifier::Generic& id,
        const identifier::Nym& nym) noexcept = 0;
    virtual void Unregister(const identifier::Generic& id) noexcept = 0;
    virtual auto UpgradeNymVersion(const VersionNumber nymVersion) noexcept
        -> bool = 0;

    ~Group() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
