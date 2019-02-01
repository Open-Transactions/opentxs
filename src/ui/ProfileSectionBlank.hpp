// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ProfileSection.hpp"
#include "opentxs/ui/Widget.hpp"

#include "InternalUI.hpp"

namespace opentxs::ui::implementation
{
class ProfileSectionBlank final : public ProfileRowInternal
{
public:
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override
    {
        return false;
    }
    bool Delete(const int, const std::string&) const override { return false; }
    OTUIProfileSubsection First() const override
    {
        const std::shared_ptr<const ui::ProfileSubsection> empty;

        return OTUIProfileSubsection{empty};
    }
    ItemTypeList Items(const std::string&) const override { return {}; }
    bool Last() const override { return true; }
    std::string Name(const std::string& lang) const override { return {}; }
    OTUIProfileSubsection Next() const override
    {
        const std::shared_ptr<const ui::ProfileSubsection> empty;

        return OTUIProfileSubsection{empty};
    }
    bool SetActive(const int, const std::string&, const bool) const override
    {
        return false;
    }
    bool SetPrimary(const int, const std::string&, const bool) const override
    {
        return false;
    }
    bool SetValue(const int, const std::string&, const std::string&)
        const override
    {
        return false;
    }
    proto::ContactSectionName Type() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(
        const implementation::ProfileSortKey& key,
        const implementation::CustomData& custom) override
    {
    }
    bool last(const ProfileSectionRowID&) const override { return false; }
    const identifier::Nym& NymID() const override { return nym_id_; }

    ProfileSectionBlank() = default;
    ~ProfileSectionBlank() = default;

private:
    const OTNymID nym_id_{identifier::Nym::Factory()};

    ProfileSectionBlank(const ProfileSectionBlank&) = delete;
    ProfileSectionBlank(ProfileSectionBlank&&) = delete;
    ProfileSectionBlank& operator=(const ProfileSectionBlank&) = delete;
    ProfileSectionBlank& operator=(ProfileSectionBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
