// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::Attribute
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType
// IWYU pragma: no_include "opentxs/identity/wot/claim/Attribute.hpp"

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class ContactItem;
}  // namespace proto

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::claim
{
class OPENTXS_EXPORT Item
{
public:
    auto operator==(const Item& rhs) const -> bool;

    auto End() const -> const Time&;
    auto ID() const -> const identifier::Generic&;
    auto isActive() const -> bool;
    auto isLocal() const -> bool;
    auto isPrimary() const -> bool;
    auto Section() const -> const claim::SectionType&;
    auto Serialize(Writer&& destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::ContactItem& out,
        const bool withID = false) const -> bool;
    auto SetActive(const bool active) const -> Item;
    auto SetEnd(const Time end) const -> Item;
    auto SetLocal(const bool local) const -> Item;
    auto SetPrimary(const bool primary) const -> Item;
    auto SetStart(const Time start) const -> Item;
    auto SetValue(const UnallocatedCString& value) const -> Item;
    auto Start() const -> const Time&;
    auto Subtype() const -> const UnallocatedCString&;
    auto Type() const -> const claim::ClaimType&;
    auto Value() const -> const UnallocatedCString&;
    auto Version() const -> VersionNumber;

    Item(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const claim::ClaimType& type,
        const UnallocatedCString& value,
        const UnallocatedSet<claim::Attribute>& attributes,
        const Time start,
        const Time end,
        const UnallocatedCString subtype);
    Item(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const Claim& claim);
    OPENTXS_NO_EXPORT Item(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const proto::ContactItem& serialized);
    Item(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const ReadView& serialized);
    Item() = delete;
    Item(const Item&) noexcept;
    Item(Item&&) noexcept;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;

    ~Item();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::identity::wot::claim
