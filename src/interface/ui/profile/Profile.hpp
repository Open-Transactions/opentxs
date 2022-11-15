// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType

#pragma once

#include <mutex>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Wallet;
}  // namespace session
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ProfileList = List<
    ProfileExternalInterface,
    ProfileInternalInterface,
    ProfileRowID,
    ProfileRowInterface,
    ProfileRowInternal,
    ProfileRowBlank,
    ProfileSortKey,
    ProfilePrimaryID>;

class Profile final : public ProfileList
{
public:
    const api::session::Client& api_;

    auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool final;
    auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const UnallocatedCString& lang) const noexcept -> ItemTypeList final;
    auto AllowedSections(const UnallocatedCString& lang) const noexcept
        -> SectionTypeList final;
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ClearCallbacks() const noexcept -> void final;
    auto Delete(
        const int section,
        const int type,
        const UnallocatedCString& claimID) const noexcept -> bool final;
    auto DisplayName() const noexcept -> UnallocatedCString final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto ID() const noexcept -> UnallocatedCString final
    {
        return primary_id_.asBase58(api_.Crypto());
    }
    auto PaymentCode() const noexcept -> UnallocatedCString final;
    auto SetActive(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool final;
    auto SetPrimary(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool final;
    auto SetValue(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool final;

    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    Profile(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    Profile() = delete;
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    auto operator=(const Profile&) -> Profile& = delete;
    auto operator=(Profile&&) -> Profile& = delete;

    ~Profile() final;

private:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    const ListenerDefinitions listeners_;
    mutable CallbackHolder callbacks_;
    UnallocatedCString name_;
    UnallocatedCString payment_code_;

    static const UnallocatedSet<identity::wot::claim::SectionType>
        allowed_types_;
    static const UnallocatedMap<identity::wot::claim::SectionType, int>
        sort_keys_;

    static auto sort_key(const identity::wot::claim::SectionType type) noexcept
        -> int;
    static auto check_type(
        const identity::wot::claim::SectionType type) noexcept -> bool;
    static auto nym_name(
        const api::session::Crypto& crypto,
        const api::session::Wallet& wallet,
        const identifier::Nym& nymID) noexcept -> UnallocatedCString;

    auto construct_row(
        const ProfileRowID& id,
        const ProfileSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ProfileRowID& id) const noexcept -> bool final
    {
        return ProfileList::last(id);
    }

    auto process_nym(const identity::Nym& nym) noexcept -> void;
    auto process_nym(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
