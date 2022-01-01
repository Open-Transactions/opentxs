// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/contact/SectionType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/contact/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs
{
namespace contact
{
class ContactItem;
}  // namespace contact

namespace proto
{
class ContactSection;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::contact
{
class OPENTXS_EXPORT ContactGroup
{
public:
    using ItemMap = std::pmr::map<OTIdentifier, std::shared_ptr<ContactItem>>;

    ContactGroup(
        const std::string& nym,
        const contact::SectionType section,
        const contact::ClaimType type,
        const ItemMap& items);
    ContactGroup(
        const std::string& nym,
        const contact::SectionType section,
        const std::shared_ptr<ContactItem>& item);
    ContactGroup(const ContactGroup&) noexcept;
    ContactGroup(ContactGroup&&) noexcept;

    auto operator+(const ContactGroup& rhs) const -> ContactGroup;

    auto begin() const -> ItemMap::const_iterator;
    auto Best() const -> std::shared_ptr<ContactItem>;
    auto Claim(const Identifier& item) const -> std::shared_ptr<ContactItem>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto AddItem(const std::shared_ptr<ContactItem>& item) const
        -> ContactGroup;
    auto AddPrimary(const std::shared_ptr<ContactItem>& item) const
        -> ContactGroup;
    auto Delete(const Identifier& id) const -> ContactGroup;
    auto end() const -> ItemMap::const_iterator;
    auto Primary() const -> const Identifier&;
    auto PrimaryClaim() const -> std::shared_ptr<ContactItem>;
    auto SerializeTo(proto::ContactSection& section, const bool withIDs = false)
        const -> bool;
    auto Size() const -> std::size_t;
    auto Type() const -> const contact::ClaimType&;

    ~ContactGroup();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactGroup() = delete;
    auto operator=(const ContactGroup&) -> ContactGroup& = delete;
    auto operator=(ContactGroup&&) -> ContactGroup& = delete;
};
}  // namespace opentxs::contact
