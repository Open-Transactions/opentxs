// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileSectionList = List<
    ProfileSectionExternalInterface,
    ProfileSectionInternalInterface,
    ProfileSectionRowID,
    ProfileSectionRowInterface,
    ProfileSectionRowInternal,
    ProfileSectionRowBlank,
    ProfileSectionSortKey>;
using ProfileSectionRow =
    RowType<ProfileRowInternal, ProfileInternalInterface, ProfileRowID>;

class ProfileSection final : public ProfileSectionList, public ProfileSectionRow
{
public:
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override;
    bool Delete(const int type, const std::string& claimID) const override;
    ItemTypeList Items(const std::string& lang) const override;
    std::string Name(const std::string& lang) const override;
    const identifier::Nym& NymID() const override { return nym_id_; }
    bool SetActive(
        const int type,
        const std::string& claimID,
        const bool active) const override;
    bool SetPrimary(
        const int type,
        const std::string& claimID,
        const bool primary) const override;
    bool SetValue(
        const int type,
        const std::string& claimID,
        const std::string& value) const override;
    proto::ContactSectionName Type() const override { return row_id_; }

    void reindex(const ProfileSortKey& key, const CustomData& custom) override;

    ~ProfileSection() = default;

private:
    friend opentxs::Factory;

    static int sort_key(const ProfileSectionRowID type);
    static bool check_type(const ProfileSectionRowID type);

    void construct_row(
        const ProfileSectionRowID& id,
        const ProfileSectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileSectionRowID& id) const override
    {
        return ProfileSectionList::last(id);
    }
    std::set<ProfileSectionRowID> process_section(
        const opentxs::ContactSection& section);
    void startup(const CustomData& custom);

    ProfileSection(
        const ProfileInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ProfileRowID& rowID,
        const ProfileSortKey& key,
        const CustomData& custom);
    ProfileSection() = delete;
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    ProfileSection& operator=(const ProfileSection&) = delete;
    ProfileSection& operator=(ProfileSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
