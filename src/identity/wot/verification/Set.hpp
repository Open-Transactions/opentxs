// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/identity/wot/verification/Group.hpp"
#include "internal/identity/wot/verification/Item.hpp"
#include "internal/identity/wot/verification/Set.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Group.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::implementation
{
class Set final : public internal::Set
{
public:
    operator SerializedType() const noexcept final;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto External() const noexcept -> const verification::Group& final
    {
        return *external_;
    }
    auto Internal() const noexcept -> const verification::Group& final
    {
        return *internal_;
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const identifier::Nym& claimOwner,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const verification::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(
        const identifier::Nym& verifier,
        const internal::Item::SerializedType verification) noexcept
        -> bool final;
    auto DeleteItem(const identifier::Generic& item) noexcept -> bool final;
    auto External() noexcept -> verification::Group& final
    {
        return *external_;
    }
    auto Internal() noexcept -> verification::Group& final
    {
        return *internal_;
    }
    void Register(const identifier::Generic& id, const bool external) noexcept
        final;
    void Unregister(const identifier::Generic& id) noexcept final;
    auto UpgradeGroupVersion(const VersionNumber groupVersion) noexcept
        -> bool final;

    Set() = delete;
    Set(const Set&) = delete;
    Set(Set&&) = delete;
    auto operator=(const Set&) -> Set& = delete;
    auto operator=(Set&&) -> Set& = delete;

    ~Set() final = default;

private:
    friend opentxs::Factory;

    using GroupPointer = std::unique_ptr<internal::Group>;
    using ChildType = internal::Group::SerializedType;

    const api::Session& api_;
    const VersionNumber version_;
    const identifier::Nym nym_id_;
    GroupPointer internal_;
    GroupPointer external_;
    UnallocatedMap<identifier::Generic, bool> map_;

    static auto instantiate(
        internal::Set& parent,
        const ChildType& serialized,
        bool external) noexcept -> GroupPointer;

    Set(const api::Session& api,
        const identifier::Nym& nym,
        const VersionNumber version = DefaultVersion) noexcept(false);
    Set(const api::Session& api,
        const identifier::Nym& nym,
        const SerializedType& serialized) noexcept(false);
};
}  // namespace opentxs::identity::wot::verification::implementation
