// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageContacts.pb.h>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace proto
{
class Contact;
}  // namespace proto

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Trunk;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Contacts final : public Node
{
public:
    auto Alias(const identifier::Generic& id) const -> UnallocatedCString;
    auto List() const -> ObjectList final;
    auto Load(
        const identifier::Generic& id,
        std::shared_ptr<proto::Contact>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;
    auto NymOwner(const identifier::Nym& nym) const -> identifier::Generic;
    auto Save() const -> bool;

    auto Delete(const identifier::Generic& id) -> bool;
    auto SetAlias(const identifier::Generic& id, std::string_view alias)
        -> bool;
    auto Store(const proto::Contact& data, std::string_view alias) -> bool;

    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts = delete;
    auto operator=(Contacts&&) -> Contacts = delete;

    ~Contacts() final = default;

private:
    friend Trunk;
    using ot_super = Node;
    using Address =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber MergeIndexVersion{1};
    static const VersionNumber NymIndexVersion{1};

    Map<identifier::Generic, Set<identifier::Generic>> merge_;
    Map<identifier::Generic, identifier::Generic> merged_;
    mutable Map<identifier::Nym, identifier::Generic> nym_contact_index_;

    void extract_nyms(const Lock& lock, const proto::Contact& data) const;
    auto nomalize_id(const identifier::Generic& input) const
        -> const identifier::Generic&;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageContacts;

    auto init(const Hash& hash) noexcept(false) -> void final;
    void reconcile_maps(const Lock& lock, const proto::Contact& data);
    void reverse_merged();
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Contacts(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
