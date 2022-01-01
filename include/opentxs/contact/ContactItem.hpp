// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#define NULL_START 0
#define NULL_END 0

// IWYU pragma: no_include "opentxs/contact/Attribute.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <set>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Identifier;

namespace proto
{
class ContactItem;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::contact
{
class OPENTXS_EXPORT ContactItem
{
public:
    ContactItem(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const contact::SectionType section,
        const contact::ClaimType& type,
        const std::string& value,
        const std::pmr::set<contact::Attribute>& attributes,
        const std::time_t start,
        const std::time_t end,
        const std::string subtype);
    ContactItem(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const Claim& claim);
    OPENTXS_NO_EXPORT ContactItem(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const contact::SectionType section,
        const proto::ContactItem& serialized);
    ContactItem(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const contact::SectionType section,
        const ReadView& serialized);
    ContactItem(const ContactItem&) noexcept;
    ContactItem(ContactItem&&) noexcept;

    auto operator==(const ContactItem& rhs) const -> bool;

    auto End() const -> const std::time_t&;
    auto ID() const -> const Identifier&;
    auto isActive() const -> bool;
    auto isLocal() const -> bool;
    auto isPrimary() const -> bool;
    auto Section() const -> const contact::SectionType&;
    auto Serialize(AllocateOutput destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::ContactItem& out,
        const bool withID = false) const -> bool;
    auto SetActive(const bool active) const -> ContactItem;
    auto SetEnd(const std::time_t end) const -> ContactItem;
    auto SetLocal(const bool local) const -> ContactItem;
    auto SetPrimary(const bool primary) const -> ContactItem;
    auto SetStart(const std::time_t start) const -> ContactItem;
    auto SetValue(const std::string& value) const -> ContactItem;
    auto Start() const -> const std::time_t&;
    auto Subtype() const -> const std::string&;
    auto Type() const -> const contact::ClaimType&;
    auto Value() const -> const std::string&;
    auto Version() const -> VersionNumber;

    ~ContactItem();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactItem() = delete;
    auto operator=(const ContactItem&) -> ContactItem& = delete;
    auto operator=(ContactItem&&) -> ContactItem& = delete;
};
}  // namespace opentxs::contact
