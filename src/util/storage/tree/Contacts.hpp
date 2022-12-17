// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageContacts.pb.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
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

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class Contact;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Contacts final : public Node
{
public:
    auto Alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto List() const -> ObjectList final;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::Contact>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;
    auto NymOwner(const identifier::Nym& nym) const -> identifier::Generic;
    auto Save() const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto Store(const proto::Contact& data, const UnallocatedCString& alias)
        -> bool;

    Contacts() = delete;
    Contacts(const Contacts&) = delete;
    Contacts(Contacts&&) = delete;
    auto operator=(const Contacts&) -> Contacts = delete;
    auto operator=(Contacts&&) -> Contacts = delete;

    ~Contacts() final = default;

private:
    friend Tree;
    using ot_super = Node;
    using Address =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;

    static const VersionNumber CurrentVersion{2};
    static const VersionNumber MergeIndexVersion{1};
    static const VersionNumber NymIndexVersion{1};

    Map<UnallocatedCString, Set<UnallocatedCString>> merge_;
    Map<UnallocatedCString, UnallocatedCString> merged_;
    mutable Map<identifier::Nym, identifier::Generic> nym_contact_index_;

    void extract_nyms(const Lock& lock, const proto::Contact& data) const;
    auto nomalize_id(const UnallocatedCString& input) const
        -> const UnallocatedCString&;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageContacts;

    void init(const UnallocatedCString& hash) final;
    void reconcile_maps(const Lock& lock, const proto::Contact& data);
    void reverse_merged();

    Contacts(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash);
};
}  // namespace opentxs::storage
