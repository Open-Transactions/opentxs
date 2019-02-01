// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileList = List<
    ProfileExternalInterface,
    ProfileInternalInterface,
    ProfileRowID,
    ProfileRowInterface,
    ProfileRowInternal,
    ProfileRowBlank,
    ProfileSortKey>;

class Profile final : public ProfileList
{
public:
    bool AddClaim(
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override;
    ItemTypeList AllowedItems(
        const proto::ContactSectionName section,
        const std::string& lang) const override;
    SectionTypeList AllowedSections(const std::string& lang) const override;
    bool Delete(const int section, const int type, const std::string& claimID)
        const override;
    std::string DisplayName() const override;
    const identifier::Nym& NymID() const override;
    std::string ID() const override { return nym_id_->str(); }
    std::string PaymentCode() const override;
    bool SetActive(
        const int section,
        const int type,
        const std::string& claimID,
        const bool active) const override;
    bool SetPrimary(
        const int section,
        const int type,
        const std::string& claimID,
        const bool primary) const override;
    bool SetValue(
        const int section,
        const int type,
        const std::string& claimID,
        const std::string& value) const override;

    ~Profile();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    std::string name_;
    std::string payment_code_;

    static const std::set<proto::ContactSectionName> allowed_types_;
    static const std::map<proto::ContactSectionName, int> sort_keys_;

    static int sort_key(const proto::ContactSectionName type);
    static bool check_type(const proto::ContactSectionName type);
    static std::string nym_name(
        const api::Wallet& wallet,
        const identifier::Nym& nymID);

    void construct_row(
        const ProfileRowID& id,
        const ProfileSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileRowID& id) const override
    {
        return ProfileList::last(id);
    }

    void process_nym(const Nym& nym);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    Profile(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID);
    Profile() = delete;
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    Profile& operator=(const Profile&) = delete;
    Profile& operator=(Profile&&) = delete;
};
}  // namespace opentxs::ui::implementation
