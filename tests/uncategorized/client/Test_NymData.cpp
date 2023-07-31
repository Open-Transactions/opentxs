// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <ankerl/unordered_dense.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <iterator>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "ottest/fixtures/client/NymData.hpp"

namespace ot = opentxs;

namespace ottest
{
static const ot::UnallocatedCString paymentCode{
    "PM8TJKxypQfFUaHfSq59nn82EjdGU4SpHcp2ssa4GxPshtzoFtmnjfoRuHpvLiyASD7itH6auP"
    "C66jekGjnqToqS9ZJWWdf1c9L8x4iaFCQ2Gq5hMEFC"};

TEST_F(NymData, AddClaim)
{
    static constexpr auto attrib = {
        ot::identity::wot::claim::Attribute::Active};
    const auto claim = client_.Factory().Claim(
        nym_data_.Nym().ID(),
        ot::identity::wot::claim::SectionType::Contract,
        ot::identity::wot::claim::ClaimType::Usd,
        "claimValue",
        {},
        attrib);
    auto added = nym_data_.AddClaim(claim, reason_);

    EXPECT_TRUE(added);
}

TEST_F(NymData, AddContract)
{
    auto added =
        nym_data_.AddContract("", ot::UnitType::Usd, false, false, reason_);
    EXPECT_FALSE(added);

    const auto identifier1(client_.Factory().UnitIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            {},
            {},
            "instrumentDefinitionID1",
            "")));

    added = nym_data_.AddContract(
        identifier1.asBase58(client_.Crypto()),
        ot::UnitType::Usd,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);
}

TEST_F(NymData, AddEmail)
{
    auto added = nym_data_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddEmail("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(NymData, AddPaymentCode)
{
    auto added =
        nym_data_.AddPaymentCode("", ot::UnitType::Usd, false, false, reason_);
    EXPECT_FALSE(added);

    added = nym_data_.AddPaymentCode(
        paymentCode, ot::UnitType::Usd, false, false, reason_);
    EXPECT_TRUE(added);
}

TEST_F(NymData, AddPhoneNumber)
{
    auto added = nym_data_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddPhoneNumber("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(NymData, AddPreferredOTServer)
{
    const auto identifier(client_.Factory().NotaryIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            {},
            {},
            "localhost",
            "")));

    auto added = nym_data_.AddPreferredOTServer(
        identifier.asBase58(client_.Crypto()), false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddPreferredOTServer("", false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(NymData, AddSocialMediaProfile)
{
    auto added = nym_data_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Twitter,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddSocialMediaProfile(
        "",
        ot::identity::wot::claim::ClaimType::Twitter,
        false,
        false,
        reason_);
    EXPECT_FALSE(added);
}

TEST_F(NymData, BestEmail)
{
    auto added = nym_data_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddEmail("email2", false, true, reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString email = nym_data_.BestEmail();
    // First email added is made primary.
    EXPECT_STREQ("email1", email.c_str());
}

TEST_F(NymData, BestPhoneNumber)
{
    auto added = nym_data_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddPhoneNumber("phone2", false, true, reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString phone = nym_data_.BestPhoneNumber();
    // First phone number added is made primary.
    EXPECT_STREQ("phone1", phone.c_str());
}

TEST_F(NymData, BestSocialMediaProfile)
{
    auto added = nym_data_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Yahoo,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddSocialMediaProfile(
        "profile2",
        ot::identity::wot::claim::ClaimType::Yahoo,
        false,
        true,
        reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString profile = nym_data_.BestSocialMediaProfile(
        ot::identity::wot::claim::ClaimType::Yahoo);
    // First profile added is made primary.
    EXPECT_STREQ("profile1", profile.c_str());
}

TEST_F(NymData, Claims)
{
    auto contactData = nym_data_.Claims();
    const auto expected =
        ExpectedStringOutput(nym_data_.Nym().ContactDataVersion());

    ot::UnallocatedCString output = contactData;
    EXPECT_TRUE(!output.empty());
    EXPECT_STREQ(expected.c_str(), output.c_str());
}

TEST_F(NymData, DeleteClaim)
{
    static constexpr auto attrib = {
        ot::identity::wot::claim::Attribute::Active};
    const auto claim = client_.Factory().Claim(
        nym_data_.Nym().ID(),
        ot::identity::wot::claim::SectionType::Contract,
        ot::identity::wot::claim::ClaimType::Usd,
        "claimValue",
        {},
        attrib);
    auto added = nym_data_.AddClaim(claim, reason_);

    ASSERT_TRUE(added);

    const auto identifier(client_.Factory().UnitIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            {},
            {},
            "claimValue",
            "")));
    auto deleted = nym_data_.DeleteClaim(identifier, reason_);

    EXPECT_TRUE(deleted);
}

TEST_F(NymData, EmailAddresses)
{
    auto added = nym_data_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddEmail("email2", false, false, reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddEmail("email3", true, false, reason_);
    EXPECT_TRUE(added);

    auto emails = nym_data_.EmailAddresses(false);
    EXPECT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email2") != ot::UnallocatedCString::npos &&
        emails.find("email3") != ot::UnallocatedCString::npos);

    emails = nym_data_.EmailAddresses();
    // First email added is made primary and active.
    EXPECT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(emails.find("email2") == ot::UnallocatedCString::npos);
}

TEST_F(NymData, HaveContract)
{
    const auto identifier1(client_.Factory().UnitIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            {},
            {},
            "instrumentDefinitionID1",
            "")));

    auto added = nym_data_.AddContract(
        identifier1.asBase58(client_.Crypto()),
        ot::UnitType::Usd,
        false,
        false,
        reason_);
    ASSERT_TRUE(added);

    auto haveContract =
        nym_data_.HaveContract(identifier1, ot::UnitType::Usd, true, true);
    EXPECT_TRUE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier1, ot::UnitType::Usd, true, false);
    EXPECT_TRUE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier1, ot::UnitType::Usd, false, true);
    EXPECT_TRUE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier1, ot::UnitType::Usd, false, false);
    EXPECT_TRUE(haveContract);

    const auto identifier2(client_.Factory().UnitIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            {},
            {},
            "instrumentDefinitionID2",
            "")));

    added = nym_data_.AddContract(
        identifier2.asBase58(client_.Crypto()),
        ot::UnitType::Usd,
        false,
        false,
        reason_);
    ASSERT_TRUE(added);

    haveContract =
        nym_data_.HaveContract(identifier2, ot::UnitType::Usd, false, false);
    EXPECT_TRUE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier2, ot::UnitType::Usd, true, false);
    EXPECT_FALSE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier2, ot::UnitType::Usd, false, true);
    EXPECT_FALSE(haveContract);

    haveContract =
        nym_data_.HaveContract(identifier2, ot::UnitType::Usd, true, true);
    EXPECT_FALSE(haveContract);
}

TEST_F(NymData, Name) { EXPECT_STREQ("testNym", nym_data_.Name().c_str()); }

TEST_F(NymData, Nym)
{
    EXPECT_STREQ("testNym", nym_data_.Nym().Name().c_str());
}

TEST_F(NymData, PaymentCode)
{
    auto added = nym_data_.AddPaymentCode(
        paymentCode, ot::UnitType::Btc, true, true, reason_);
    ASSERT_TRUE(added);

    auto paymentcode = nym_data_.PaymentCode(ot::UnitType::Btc);
    EXPECT_TRUE(!paymentcode.empty());
    EXPECT_STREQ(paymentCode.c_str(), paymentcode.c_str());

    paymentcode = nym_data_.PaymentCode(ot::UnitType::Usd);
    EXPECT_TRUE(paymentcode.empty());
}

TEST_F(NymData, PhoneNumbers)
{
    auto added = nym_data_.AddPhoneNumber("phone1", false, false, reason_);
    ASSERT_TRUE(added);

    added = nym_data_.AddPhoneNumber("phone2", false, false, reason_);
    ASSERT_TRUE(added);

    added = nym_data_.AddPhoneNumber("phone3", true, false, reason_);
    ASSERT_TRUE(added);

    auto phones = nym_data_.PhoneNumbers(false);
    EXPECT_TRUE(
        phones.find("phone1") != ot::UnallocatedCString::npos &&
        phones.find("phone2") != ot::UnallocatedCString::npos &&
        phones.find("phone3") != ot::UnallocatedCString::npos);

    phones = nym_data_.PhoneNumbers();
    // First phone number added is made primary and active.
    EXPECT_TRUE(
        phones.find("phone1") != ot::UnallocatedCString::npos &&
        phones.find("phone3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(phones.find("phone2") == ot::UnallocatedCString::npos);
}

TEST_F(NymData, PreferredOTServer)
{
    auto preferred = nym_data_.PreferredOTServer();
    EXPECT_TRUE(preferred.empty());

    const auto identifier(client_.Factory().NotaryIDFromBase58(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            {},
            {},
            "localhost",
            "")));
    auto added = nym_data_.AddPreferredOTServer(
        identifier.asBase58(client_.Crypto()), true, reason_);
    EXPECT_TRUE(added);

    preferred = nym_data_.PreferredOTServer();
    EXPECT_TRUE(!preferred.empty());
    EXPECT_STREQ(
        identifier.asBase58(client_.Crypto()).c_str(), preferred.c_str());
}

TEST_F(NymData, PrintContactData)
{
    const auto& text = nym_data_.PrintContactData();
    const auto expected =
        ExpectedStringOutput(nym_data_.Nym().ContactDataVersion());

    EXPECT_STREQ(expected.c_str(), text.c_str());
}

TEST_F(NymData, SetContactData)
{
    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(client_),
        ot::UnallocatedCString("contactData"),
        nym_data_.Nym().ContactDataVersion(),
        nym_data_.Nym().ContactDataVersion(),
        {});

    auto bytes = ot::Space{};
    EXPECT_TRUE(contactData.Serialize(ot::writer(bytes)));
    auto set = nym_data_.SetContactData(ot::reader(bytes), reason_);
    EXPECT_TRUE(set);
}

TEST_F(NymData, SetScope)
{
    auto set = nym_data_.SetScope(
        ot::identity::wot::claim::ClaimType::Organization,
        "organizationScope",
        true,
        reason_);
    EXPECT_TRUE(set);

    set = nym_data_.SetScope(
        ot::identity::wot::claim::ClaimType::Business,
        "businessScope",
        false,
        reason_);
    EXPECT_TRUE(set);
}

TEST_F(NymData, SocialMediaProfiles)
{
    auto added = nym_data_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddSocialMediaProfile(
        "profile2",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nym_data_.AddSocialMediaProfile(
        "profile3",
        ot::identity::wot::claim::ClaimType::Facebook,
        true,
        false,
        reason_);
    EXPECT_TRUE(added);

    auto profiles = nym_data_.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook, false);
    EXPECT_TRUE(
        profiles.find("profile1") != ot::UnallocatedCString::npos &&
        profiles.find("profile2") != ot::UnallocatedCString::npos &&
        profiles.find("profile3") != ot::UnallocatedCString::npos);

    profiles = nym_data_.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook);
    // First profile added is made primary and active.
    EXPECT_TRUE(
        profiles.find("profile1") != ot::UnallocatedCString::npos &&
        profiles.find("profile3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(profiles.find("profile2") == ot::UnallocatedCString::npos);
}

TEST_F(NymData, SocialMediaProfileTypes)
{
    ankerl::unordered_dense::set<ot::proto::ContactItemType> profileTypes =
        ot::proto::AllowedItemTypes().at(ot::proto::ContactSectionVersion(
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            translate(ot::identity::wot::claim::SectionType::Profile)));

    ot::UnallocatedSet<ot::identity::wot::claim::ClaimType> output;
    std::transform(
        profileTypes.begin(),
        profileTypes.end(),
        std::inserter(output, output.end()),
        [](ot::proto::ContactItemType itemtype)
            -> ot::identity::wot::claim::ClaimType {
            return translate(itemtype);
        });

    EXPECT_EQ(output, nym_data_.SocialMediaProfileTypes());
}

TEST_F(NymData, Type)
{
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Individual, nym_data_.Type());
}

TEST_F(NymData, Valid) { EXPECT_TRUE(nym_data_.Valid()); }
}  // namespace ottest
