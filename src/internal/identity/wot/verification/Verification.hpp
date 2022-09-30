// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/verification/Group.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/identity/wot/verification/Set.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::internal
{
struct Group : virtual public verification::Group {
    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto External() const noexcept -> bool = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    virtual void Register(
        const identifier::Generic& id,
        const identifier::Nym& nym) noexcept = 0;
    virtual void Unregister(const identifier::Generic& id) noexcept = 0;
    virtual auto UpgradeNymVersion(const VersionNumber nymVersion) noexcept
        -> bool = 0;

    ~Group() override = default;
};
struct Item : virtual public verification::Item {
    ~Item() override = default;
};
struct Nym : virtual public verification::Nym {
    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    using verification::Nym::AddItem;
    virtual auto AddItem(const Item::SerializedType item) noexcept -> bool = 0;
    virtual auto UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept -> bool = 0;

    ~Nym() override = default;
};
struct Set : virtual public verification::Set {
    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    virtual void Register(
        const identifier::Generic& id,
        const bool external) noexcept = 0;
    virtual void Unregister(const identifier::Generic& id) noexcept = 0;
    virtual auto UpgradeGroupVersion(const VersionNumber groupVersion) noexcept
        -> bool = 0;

    ~Set() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
