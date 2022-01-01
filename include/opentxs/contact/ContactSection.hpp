// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/contact/SectionType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace contact
{
class ContactGroup;
class ContactItem;
}  // namespace contact

namespace proto
{
class ContactData;
class ContactSection;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::contact
{
class OPENTXS_EXPORT ContactSection
{
public:
    using GroupMap =
        std::pmr::map<contact::ClaimType, std::shared_ptr<ContactGroup>>;

    ContactSection(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const contact::SectionType section,
        const GroupMap& groups);
    ContactSection(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const contact::SectionType section,
        const std::shared_ptr<ContactItem>& item);
    OPENTXS_NO_EXPORT ContactSection(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const proto::ContactSection& serialized);
    ContactSection(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const ReadView& serialized);
    ContactSection(const ContactSection&) noexcept;
    ContactSection(ContactSection&&) noexcept;

    auto operator+(const ContactSection& rhs) const -> ContactSection;

    auto AddItem(const std::shared_ptr<ContactItem>& item) const
        -> ContactSection;
    auto begin() const -> GroupMap::const_iterator;
    auto Claim(const Identifier& item) const -> std::shared_ptr<ContactItem>;
    auto Delete(const Identifier& id) const -> ContactSection;
    auto end() const -> GroupMap::const_iterator;
    auto Group(const contact::ClaimType& type) const
        -> std::shared_ptr<ContactGroup>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto Serialize(AllocateOutput destination, const bool withIDs = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto SerializeTo(
        proto::ContactData& data,
        const bool withIDs = false) const -> bool;
    auto Size() const -> std::size_t;
    auto Type() const -> const contact::SectionType&;
    auto Version() const -> VersionNumber;

    ~ContactSection();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactSection() = delete;
    auto operator=(const ContactSection&) -> ContactSection& = delete;
    auto operator=(ContactSection&&) -> ContactSection& = delete;
};
}  // namespace opentxs::contact
