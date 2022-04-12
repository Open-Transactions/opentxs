// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <tuple>

#include "1_Internal.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_NymData : public ::testing::Test
{
public:
    const ot::api::session::Client& client_;
    ot::OTPasswordPrompt reason_;
    ot::NymData nymData_;

    static ot::UnallocatedCString ExpectedStringOutput(
        const std::uint32_t version)
    {
        return ot::UnallocatedCString{"Version "} + std::to_string(version) +
               ot::UnallocatedCString(
                   " contact data\nSections found: 1\n- Section: "
                   "Scope, version: ") +
               std::to_string(version) +
               ot::UnallocatedCString{
                   " containing 1 item(s).\n-- Item type: "
                   "\"Individual\", value: "
                   "\"testNym\", start: 0, end: 0, version: "} +
               std::to_string(version) +
               ot::UnallocatedCString{"\n--- Attributes: Active Primary \n"};
    }

    Test_NymData()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        , nymData_(client_.Wallet().mutable_Nym(
              client_.Wallet().Nym(reason_, "testNym")->ID(),
              reason_))
    {
    }
};

static const ot::UnallocatedCString paymentCode{
    "PM8TJKxypQfFUaHfSq59nn82EjdGU4SpHcp2ssa4GxPshtzoFtmnjfoRuHpvLiyASD7itH6auP"
    "C66jekGjnqToqS9ZJWWdf1c9L8x4iaFCQ2Gq5hMEFC"};

TEST_F(Test_NymData, AddClaim)
{
    ot::Claim claim = std::make_tuple(
        ot::UnallocatedCString(""),
        ot::translate(ot::identity::wot::claim::SectionType::Contract),
        ot::translate(ot::identity::wot::claim::ClaimType::Usd),
        ot::UnallocatedCString("claimValue"),
        NULL_START,
        NULL_END,
        ot::UnallocatedSet<std::uint32_t>{static_cast<uint32_t>(
            ot::identity::wot::claim::Attribute::Active)});

    auto added = nymData_.AddClaim(claim, reason_);
    EXPECT_TRUE(added);
}

TEST_F(Test_NymData, AddContract)
{
    auto added =
        nymData_.AddContract("", ot::UnitType::Usd, false, false, reason_);
    EXPECT_FALSE(added);

    const auto identifier1(ot::identifier::UnitDefinition::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID1",
            "")));

    added = nymData_.AddContract(
        identifier1->str(), ot::UnitType::Usd, false, false, reason_);
    EXPECT_TRUE(added);
}

TEST_F(Test_NymData, AddEmail)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, AddPaymentCode)
{
    auto added =
        nymData_.AddPaymentCode("", ot::UnitType::Usd, false, false, reason_);
    EXPECT_FALSE(added);

    added = nymData_.AddPaymentCode(
        paymentCode, ot::UnitType::Usd, false, false, reason_);
    EXPECT_TRUE(added);
}

TEST_F(Test_NymData, AddPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPhoneNumber("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, AddPreferredOTServer)
{
    const auto identifier(ot::identifier::Notary::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            "localhost",
            "")));

    auto added =
        nymData_.AddPreferredOTServer(identifier->str(), false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPreferredOTServer("", false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, AddSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Twitter,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "",
        ot::identity::wot::claim::ClaimType::Twitter,
        false,
        false,
        reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, BestEmail)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email2", false, true, reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString email = nymData_.BestEmail();
    // First email added is made primary.
    EXPECT_STREQ("email1", email.c_str());
}

TEST_F(Test_NymData, BestPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, true, reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString phone = nymData_.BestPhoneNumber();
    // First phone number added is made primary.
    EXPECT_STREQ("phone1", phone.c_str());
}

TEST_F(Test_NymData, BestSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Yahoo,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2",
        ot::identity::wot::claim::ClaimType::Yahoo,
        false,
        true,
        reason_);
    EXPECT_TRUE(added);

    ot::UnallocatedCString profile = nymData_.BestSocialMediaProfile(
        ot::identity::wot::claim::ClaimType::Yahoo);
    // First profile added is made primary.
    EXPECT_STREQ("profile1", profile.c_str());
}

TEST_F(Test_NymData, Claims)
{
    auto contactData = nymData_.Claims();
    const auto expected =
        ExpectedStringOutput(nymData_.Nym().ContactDataVersion());

    ot::UnallocatedCString output = contactData;
    EXPECT_TRUE(!output.empty());
    EXPECT_STREQ(expected.c_str(), output.c_str());
}

TEST_F(Test_NymData, DeleteClaim)
{
    ot::Claim claim = std::make_tuple(
        ot::UnallocatedCString(""),
        ot::translate(ot::identity::wot::claim::SectionType::Contract),
        ot::translate(ot::identity::wot::claim::ClaimType::Usd),
        ot::UnallocatedCString("claimValue"),
        NULL_START,
        NULL_END,
        ot::UnallocatedSet<std::uint32_t>{static_cast<uint32_t>(
            ot::identity::wot::claim::Attribute::Active)});

    auto added = nymData_.AddClaim(claim, reason_);
    ASSERT_TRUE(added);

    const auto identifier(ot::identifier::UnitDefinition::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "claimValue",
            "")));
    auto deleted = nymData_.DeleteClaim(identifier, reason_);
    EXPECT_TRUE(deleted);
}

TEST_F(Test_NymData, EmailAddresses)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email2", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email3", true, false, reason_);
    EXPECT_TRUE(added);

    auto emails = nymData_.EmailAddresses(false);
    EXPECT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email2") != ot::UnallocatedCString::npos &&
        emails.find("email3") != ot::UnallocatedCString::npos);

    emails = nymData_.EmailAddresses();
    // First email added is made primary and active.
    EXPECT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(emails.find("email2") == ot::UnallocatedCString::npos);
}

TEST_F(Test_NymData, HaveContract)
{
    const auto identifier1(ot::identifier::UnitDefinition::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID1",
            "")));

    auto added = nymData_.AddContract(
        identifier1->str(), ot::UnitType::Usd, false, false, reason_);
    ASSERT_TRUE(added);

    auto haveContract =
        nymData_.HaveContract(identifier1, ot::UnitType::Usd, true, true);
    EXPECT_TRUE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier1, ot::UnitType::Usd, true, false);
    EXPECT_TRUE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier1, ot::UnitType::Usd, false, true);
    EXPECT_TRUE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier1, ot::UnitType::Usd, false, false);
    EXPECT_TRUE(haveContract);

    const auto identifier2(ot::identifier::UnitDefinition::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID2",
            "")));

    added = nymData_.AddContract(
        identifier2->str(), ot::UnitType::Usd, false, false, reason_);
    ASSERT_TRUE(added);

    haveContract =
        nymData_.HaveContract(identifier2, ot::UnitType::Usd, false, false);
    EXPECT_TRUE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier2, ot::UnitType::Usd, true, false);
    EXPECT_FALSE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier2, ot::UnitType::Usd, false, true);
    EXPECT_FALSE(haveContract);

    haveContract =
        nymData_.HaveContract(identifier2, ot::UnitType::Usd, true, true);
    EXPECT_FALSE(haveContract);
}

TEST_F(Test_NymData, Name) { EXPECT_STREQ("testNym", nymData_.Name().c_str()); }

TEST_F(Test_NymData, Nym)
{
    EXPECT_STREQ("testNym", nymData_.Nym().Name().c_str());
}

TEST_F(Test_NymData, PaymentCode)
{
    auto added = nymData_.AddPaymentCode(
        paymentCode, ot::UnitType::Btc, true, true, reason_);
    ASSERT_TRUE(added);

    auto paymentcode = nymData_.PaymentCode(ot::UnitType::Btc);
    EXPECT_TRUE(!paymentcode.empty());
    EXPECT_STREQ(paymentCode.c_str(), paymentcode.c_str());

    paymentcode = nymData_.PaymentCode(ot::UnitType::Usd);
    EXPECT_TRUE(paymentcode.empty());
}

TEST_F(Test_NymData, PhoneNumbers)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, false, reason_);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone3", true, false, reason_);
    ASSERT_TRUE(added);

    auto phones = nymData_.PhoneNumbers(false);
    EXPECT_TRUE(
        phones.find("phone1") != ot::UnallocatedCString::npos &&
        phones.find("phone2") != ot::UnallocatedCString::npos &&
        phones.find("phone3") != ot::UnallocatedCString::npos);

    phones = nymData_.PhoneNumbers();
    // First phone number added is made primary and active.
    EXPECT_TRUE(
        phones.find("phone1") != ot::UnallocatedCString::npos &&
        phones.find("phone3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(phones.find("phone2") == ot::UnallocatedCString::npos);
}

TEST_F(Test_NymData, PreferredOTServer)
{
    auto preferred = nymData_.PreferredOTServer();
    EXPECT_TRUE(preferred.empty());

    const auto identifier(ot::identifier::Notary::Factory(
        ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(client_),
            "testNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            "localhost",
            "")));
    auto added =
        nymData_.AddPreferredOTServer(identifier->str(), true, reason_);
    EXPECT_TRUE(added);

    preferred = nymData_.PreferredOTServer();
    EXPECT_TRUE(!preferred.empty());
    EXPECT_STREQ(identifier->str().c_str(), preferred.c_str());
}

TEST_F(Test_NymData, PrintContactData)
{
    const auto& text = nymData_.PrintContactData();
    const auto expected =
        ExpectedStringOutput(nymData_.Nym().ContactDataVersion());

    EXPECT_STREQ(expected.c_str(), text.c_str());
}

TEST_F(Test_NymData, SetContactData)
{
    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(client_),
        ot::UnallocatedCString("contactData"),
        nymData_.Nym().ContactDataVersion(),
        nymData_.Nym().ContactDataVersion(),
        {});

    auto bytes = ot::Space{};
    EXPECT_TRUE(contactData.Serialize(ot::writer(bytes)));
    auto set = nymData_.SetContactData(ot::reader(bytes), reason_);
    EXPECT_TRUE(set);
}

TEST_F(Test_NymData, SetScope)
{
    auto set = nymData_.SetScope(
        ot::identity::wot::claim::ClaimType::Organization,
        "organizationScope",
        true,
        reason_);
    EXPECT_TRUE(set);

    set = nymData_.SetScope(
        ot::identity::wot::claim::ClaimType::Business,
        "businessScope",
        false,
        reason_);
    EXPECT_TRUE(set);
}

TEST_F(Test_NymData, SocialMediaProfiles)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile3",
        ot::identity::wot::claim::ClaimType::Facebook,
        true,
        false,
        reason_);
    EXPECT_TRUE(added);

    auto profiles = nymData_.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook, false);
    EXPECT_TRUE(
        profiles.find("profile1") != ot::UnallocatedCString::npos &&
        profiles.find("profile2") != ot::UnallocatedCString::npos &&
        profiles.find("profile3") != ot::UnallocatedCString::npos);

    profiles = nymData_.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook);
    // First profile added is made primary and active.
    EXPECT_TRUE(
        profiles.find("profile1") != ot::UnallocatedCString::npos &&
        profiles.find("profile3") != ot::UnallocatedCString::npos);
    EXPECT_TRUE(profiles.find("profile2") == ot::UnallocatedCString::npos);
}

TEST_F(Test_NymData, SocialMediaProfileTypes)
{
    ot::UnallocatedSet<ot::proto::ContactItemType> profileTypes =
        ot::proto::AllowedItemTypes().at(ot::proto::ContactSectionVersion(
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::translate(ot::identity::wot::claim::SectionType::Profile)));

    ot::UnallocatedSet<ot::identity::wot::claim::ClaimType> output;
    std::transform(
        profileTypes.begin(),
        profileTypes.end(),
        std::inserter(output, output.end()),
        [](ot::proto::ContactItemType itemtype)
            -> ot::identity::wot::claim::ClaimType {
            return ot::translate(itemtype);
        });

    EXPECT_EQ(output, nymData_.SocialMediaProfileTypes());
}

TEST_F(Test_NymData, Type)
{
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, nymData_.Type());
}

TEST_F(Test_NymData, Valid) { EXPECT_TRUE(nymData_.Valid()); }
}  // namespace ottest
