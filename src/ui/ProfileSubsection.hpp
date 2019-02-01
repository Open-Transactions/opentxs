// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileSubsectionList = List<
    ProfileSubsectionExternalInterface,
    ProfileSubsectionInternalInterface,
    ProfileSubsectionRowID,
    ProfileSubsectionRowInterface,
    ProfileSubsectionRowInternal,
    ProfileSubsectionRowBlank,
    ProfileSubsectionSortKey>;
using ProfileSubsectionRow = RowType<
    ProfileSectionRowInternal,
    ProfileSectionInternalInterface,
    ProfileSectionRowID>;

class ProfileSubsection final : public ProfileSubsectionList,
                                public ProfileSubsectionRow
{
public:
    bool AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const override;
    bool Delete(const std::string& claimID) const override;
    std::string Name(const std::string& lang) const override;
    const identifier::Nym& NymID() const override { return nym_id_; }
    proto::ContactSectionName Section() const override { return row_id_.first; }
    bool SetActive(const std::string& claimID, const bool active)
        const override;
    bool SetPrimary(const std::string& claimID, const bool primary)
        const override;
    bool SetValue(const std::string& claimID, const std::string& value)
        const override;
    proto::ContactItemType Type() const override { return row_id_.second; }

    void reindex(const ProfileSectionSortKey& key, const CustomData& custom)
        override;

    ~ProfileSubsection() = default;

private:
    friend opentxs::Factory;

    static bool check_type(const ProfileSubsectionRowID type);

    void construct_row(
        const ProfileSubsectionRowID& id,
        const ProfileSubsectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileSubsectionRowID& id) const override
    {
        return ProfileSubsectionList::last(id);
    }
    std::set<ProfileSubsectionRowID> process_group(
        const opentxs::ContactGroup& group);
    int sort_key(const ProfileSubsectionRowID type) const;
    void startup(const CustomData& custom);

    ProfileSubsection(
        const ProfileSectionInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const ProfileSectionRowID& rowID,
        const ProfileSectionSortKey& key,
        const CustomData& custom);
    ProfileSubsection() = delete;
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    ProfileSubsection& operator=(const ProfileSubsection&) = delete;
    ProfileSubsection& operator=(ProfileSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation
