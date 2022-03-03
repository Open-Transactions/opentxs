// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#define NULL_START 0
#define NULL_END 0

// IWYU pragma: no_include "opentxs/identity/wot/claim/Attribute.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class ContactItem;
}  // namespace proto

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::claim
{
class OPENTXS_EXPORT Item
{
public:
    Item(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const claim::ClaimType& type,
        const UnallocatedCString& value,
        const UnallocatedSet<claim::Attribute>& attributes,
        const std::time_t start,
        const std::time_t end,
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
    Item(const Item&) noexcept;
    Item(Item&&) noexcept;

    auto operator==(const Item& rhs) const -> bool;

    auto End() const -> const std::time_t&;
    auto ID() const -> const Identifier&;
    auto isActive() const -> bool;
    auto isLocal() const -> bool;
    auto isPrimary() const -> bool;
    auto Section() const -> const claim::SectionType&;
    auto Serialize(AllocateOutput destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::ContactItem& out,
        const bool withID = false) const -> bool;
    auto SetActive(const bool active) const -> Item;
    auto SetEnd(const std::time_t end) const -> Item;
    auto SetLocal(const bool local) const -> Item;
    auto SetPrimary(const bool primary) const -> Item;
    auto SetStart(const std::time_t start) const -> Item;
    auto SetValue(const UnallocatedCString& value) const -> Item;
    auto Start() const -> const std::time_t&;
    auto Subtype() const -> const UnallocatedCString&;
    auto Type() const -> const claim::ClaimType&;
    auto Value() const -> const UnallocatedCString&;
    auto Version() const -> VersionNumber;

    ~Item();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Item() = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;
};
}  // namespace opentxs::identity::wot::claim
