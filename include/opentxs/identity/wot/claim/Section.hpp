// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/identity/wot/claim/SectionType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
namespace wot
{
namespace claim
{
class Group;
class Item;
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace proto
{
class ContactData;
class ContactSection;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::identity::wot::claim
{
class OPENTXS_EXPORT Section
{
public:
    using GroupMap = std::map<claim::ClaimType, std::shared_ptr<claim::Group>>;

    Section(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const GroupMap& groups);
    Section(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const std::shared_ptr<Item>& item);
    OPENTXS_NO_EXPORT Section(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const proto::ContactSection& serialized);
    Section(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const ReadView& serialized);
    Section(const Section&) noexcept;
    Section(Section&&) noexcept;

    auto operator+(const Section& rhs) const -> Section;

    auto AddItem(const std::shared_ptr<Item>& item) const -> Section;
    auto begin() const -> GroupMap::const_iterator;
    auto Claim(const Identifier& item) const -> std::shared_ptr<Item>;
    auto Delete(const Identifier& id) const -> Section;
    auto end() const -> GroupMap::const_iterator;
    auto Group(const claim::ClaimType& type) const -> std::shared_ptr<Group>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto Serialize(AllocateOutput destination, const bool withIDs = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto SerializeTo(
        proto::ContactData& data,
        const bool withIDs = false) const -> bool;
    auto Size() const -> std::size_t;
    auto Type() const -> const claim::SectionType&;
    auto Version() const -> VersionNumber;

    ~Section();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Section() = delete;
    auto operator=(const Section&) -> Section& = delete;
    auto operator=(Section&&) -> Section& = delete;
};
}  // namespace opentxs::identity::wot::claim