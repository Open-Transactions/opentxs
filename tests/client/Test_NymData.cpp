/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

namespace
{

class Test_NymData : public ::testing::Test
{
public:
    Test_NymData()
        : nymData_(opentxs::OT::App().Wallet().mutable_Nym(
              opentxs::Identifier(opentxs::OT::App().API().Exec().CreateNymHD(
                  opentxs::proto::CITEMTYPE_INDIVIDUAL,
                  "testNym",
                  "",
                  -1))))
    {
    }

    opentxs::NymData nymData_;
};

static const auto& paymentCode = "PM8TJKxypQfFUaHfSq59nn82EjdGU4SpHcp2ssa4GxPsh"
                                 "tzoFtmnjfoRuHpvLiyASD7itH6auPC66jekGjnqToqS9Z"
                                 "JWWdf1c9L8x4iaFCQ2Gq5hMEFC";

// This needs to be updated when NYM_CONTACT_DATA_VERSION in Types.hpp changes.
static const auto& expectedStringOutput =
    "Version 4 contact data\nSections found: 1\n- Section: Scope, version: 4 "
    "containing 1 item(s).\n-- Item type: \"Individual\", value: \"testNym\", "
    "start: 0, end: 0, version: 4\n--- Attributes: Active Primary \n";

}  // namespace

TEST(NymData, Check_Version)
{
    ASSERT_EQ(4, NYM_CONTACT_DATA_VERSION)
        << "Update the assert in this test case and expectedStringOutput if "
           "NYM_CONTACT_DATA_VERSION has changed.";
}

//
// TODO: NymData::AddChildKeyCredential is broken.
//
// TEST_F(Test_NymData, AddChildKeyCredential)
//{
//    const opentxs::CredentialSet* credentialSet =
//        nymData_.Nym().GetMasterCredentialByIndex(0);
//    ASSERT_NE(nullptr, credentialSet);
//
//    opentxs::OTIdentifier masterId =
//    credentialSet->GetMasterCredential().ID();
//
//    opentxs::NymParameters nymParameters;
//    std::string id =
//        nymData_.AddChildKeyCredential(masterId, nymParameters);
//    ASSERT_TRUE(!id.empty());
//}

TEST_F(Test_NymData, AddClaim)
{
    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        std::string("claimValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});

    auto added = nymData_.AddClaim(claim);
    ASSERT_TRUE(added);
}

TEST_F(Test_NymData, AddContract)
{
    auto added =
        nymData_.AddContract("", opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_FALSE(added);

    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID1"));

    added = nymData_.AddContract(
        identifier1.str(), opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(added);
}

TEST_F(Test_NymData, AddEmail)
{
    auto added = nymData_.AddEmail("email1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddEmail("", false, false);
    ASSERT_FALSE(added);
}

TEST_F(Test_NymData, asPublicNym)
{
    auto credentialIndex = nymData_.asPublicNym();

    ASSERT_TRUE(credentialIndex.IsInitialized());
}

TEST_F(Test_NymData, AddPaymentCode)
{
    auto added = nymData_.AddPaymentCode(
        "", opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_FALSE(added);

    added = nymData_.AddPaymentCode(
        paymentCode, opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(added);
}

TEST_F(Test_NymData, AddPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("", false, false);
    ASSERT_FALSE(added);
}

TEST_F(Test_NymData, AddPreferredOTServer)
{
    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        "localhost"));

    auto added = nymData_.AddPreferredOTServer(identifier.str(), false);
    ASSERT_TRUE(added);

    added = nymData_.AddPreferredOTServer("", false);
    ASSERT_FALSE(added);
}

TEST_F(Test_NymData, AddSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_TWITTER, false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "", opentxs::proto::CITEMTYPE_TWITTER, false, false);
    ASSERT_FALSE(added);
}

TEST_F(Test_NymData, BestEmail)
{
    auto added = nymData_.AddEmail("email1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddEmail("email2", false, true);
    ASSERT_TRUE(added);

    std::string email = nymData_.BestEmail();
    // First email added is made primary.
    ASSERT_STREQ("email1", email.c_str());
}

TEST_F(Test_NymData, BestPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, true);
    ASSERT_TRUE(added);

    std::string phone = nymData_.BestPhoneNumber();
    // First phone number added is made primary.
    ASSERT_STREQ("phone1", phone.c_str());
}

TEST_F(Test_NymData, BestSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_YAHOO, false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2", opentxs::proto::CITEMTYPE_YAHOO, false, true);
    ASSERT_TRUE(added);

    std::string profile =
        nymData_.BestSocialMediaProfile(opentxs::proto::CITEMTYPE_YAHOO);
    // First profile added is made primary.
    ASSERT_STREQ("profile1", profile.c_str());
}

TEST_F(Test_NymData, Claims)
{
    auto contactData = nymData_.Claims();

    std::string output = contactData;
    ASSERT_TRUE(!output.empty());
    ASSERT_STREQ(expectedStringOutput, output.c_str());
}

TEST_F(Test_NymData, DeleteClaim)
{
    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        std::string("claimValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});

    auto added = nymData_.AddClaim(claim);
    ASSERT_TRUE(added);

    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "claimValue"));
    auto deleted = nymData_.DeleteClaim(identifier);
    ASSERT_TRUE(deleted);
}

TEST_F(Test_NymData, EmailAddresses)
{
    auto added = nymData_.AddEmail("email1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddEmail("email2", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddEmail("email3", true, false);
    ASSERT_TRUE(added);

    auto emails = nymData_.EmailAddresses(false);
    ASSERT_TRUE(
        emails.find("email1") != std::string::npos &&
        emails.find("email2") != std::string::npos &&
        emails.find("email3") != std::string::npos);

    emails = nymData_.EmailAddresses();
    // First email added is made primary and active.
    ASSERT_TRUE(
        emails.find("email1") != std::string::npos &&
        emails.find("email3") != std::string::npos);
    ASSERT_TRUE(emails.find("email2") == std::string::npos);
}

TEST_F(Test_NymData, HaveContract)
{
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID1"));

    auto added = nymData_.AddContract(
        identifier1.str(), opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(added);

    auto haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, true, true);
    ASSERT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, true, false);
    ASSERT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, false, true);
    ASSERT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(haveContract);

    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID2"));

    added = nymData_.AddContract(
        identifier2.str(), opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(added);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, false, false);
    ASSERT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, true, false);
    ASSERT_FALSE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, false, true);
    ASSERT_FALSE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, true, true);
    ASSERT_FALSE(haveContract);
}

TEST_F(Test_NymData, Name) { ASSERT_STREQ("testNym", nymData_.Name().c_str()); }

TEST_F(Test_NymData, Nym)
{
    ASSERT_STREQ("testNym", nymData_.Nym().Name().c_str());
}

TEST_F(Test_NymData, PaymentCode)
{
    auto added = nymData_.AddPaymentCode(
        paymentCode, opentxs::proto::CITEMTYPE_BTC, true, true);
    ASSERT_TRUE(added);

    auto paymentcode = nymData_.PaymentCode(opentxs::proto::CITEMTYPE_BTC);
    ASSERT_TRUE(!paymentcode.empty());
    ASSERT_STREQ(paymentCode, paymentcode.c_str());

    paymentcode = nymData_.PaymentCode(opentxs::proto::CITEMTYPE_USD);
    ASSERT_TRUE(paymentcode.empty());
}

TEST_F(Test_NymData, PhoneNumbers)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone3", true, false);
    ASSERT_TRUE(added);

    auto phones = nymData_.PhoneNumbers(false);
    ASSERT_TRUE(
        phones.find("phone1") != std::string::npos &&
        phones.find("phone2") != std::string::npos &&
        phones.find("phone3") != std::string::npos);

    phones = nymData_.PhoneNumbers();
    // First phone number added is made primary and active.
    ASSERT_TRUE(
        phones.find("phone1") != std::string::npos &&
        phones.find("phone3") != std::string::npos);
    ASSERT_TRUE(phones.find("phone2") == std::string::npos);
}

TEST_F(Test_NymData, PreferredOTServer)
{
    auto preferred = nymData_.PreferredOTServer();
    ASSERT_TRUE(preferred.empty());

    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        "localhost"));
    auto added = nymData_.AddPreferredOTServer(identifier.str(), true);
    ASSERT_TRUE(added);

    preferred = nymData_.PreferredOTServer();
    ASSERT_TRUE(!preferred.empty());
    ASSERT_STREQ(identifier.str().c_str(), preferred.c_str());
}

TEST_F(Test_NymData, PrintContactData)
{
    const auto& dataString = nymData_.PrintContactData();
    ASSERT_STREQ(expectedStringOutput, dataString.c_str());
}

TEST_F(Test_NymData, SetAlias)
{
    bool set = nymData_.SetAlias("nymDataAlias");
    ASSERT_TRUE(set);

    auto alias = nymData_.Nym().Alias();
    ASSERT_STREQ(alias.c_str(), "nymDataAlias");
}

TEST_F(Test_NymData, SetContactData)
{
    const opentxs::ContactData contactData(
        std::string("contactData"),
        NYM_CONTACT_DATA_VERSION,
        NYM_CONTACT_DATA_VERSION,
        {});

    auto data = contactData.Serialize(true);
    auto set = nymData_.SetContactData(data);
    ASSERT_TRUE(set);
}

TEST_F(Test_NymData, SetScope)
{
    auto set = nymData_.SetScope(
        opentxs::proto::CITEMTYPE_ORGANIZATION, "organizationScope", true);
    ASSERT_TRUE(set);

    set = nymData_.SetScope(
        opentxs::proto::CITEMTYPE_BUSINESS, "businessScope", false);
    ASSERT_TRUE(set);
}

TEST_F(Test_NymData, SetVerificationSet)
{
    // TODO: Add more thorough tests when there are OT classes for the proto
    // verification classes.
    opentxs::proto::VerificationSet verificationSet;
    auto added = nymData_.SetVerificationSet(verificationSet);
    ASSERT_FALSE(added);
}

TEST_F(Test_NymData, SocialMediaProfiles)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_FACEBOOK, false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2", opentxs::proto::CITEMTYPE_FACEBOOK, false, false);
    ASSERT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile3", opentxs::proto::CITEMTYPE_FACEBOOK, true, false);
    ASSERT_TRUE(added);

    auto profiles =
        nymData_.SocialMediaProfiles(opentxs::proto::CITEMTYPE_FACEBOOK, false);
    ASSERT_TRUE(
        profiles.find("profile1") != std::string::npos &&
        profiles.find("profile2") != std::string::npos &&
        profiles.find("profile3") != std::string::npos);

    profiles = nymData_.SocialMediaProfiles(opentxs::proto::CITEMTYPE_FACEBOOK);
    // First profile added is made primary and active.
    ASSERT_TRUE(
        profiles.find("profile1") != std::string::npos &&
        profiles.find("profile3") != std::string::npos);
    ASSERT_TRUE(profiles.find("profile2") == std::string::npos);
}

TEST_F(Test_NymData, SocialMediaProfileTypes)
{
    std::set<opentxs::proto::ContactItemType> profileTypes =
        opentxs::proto::AllowedItemTypes.at(
            opentxs::proto::ContactSectionVersion(
                CONTACT_CONTACT_DATA_VERSION,
                opentxs::proto::CONTACTSECTION_PROFILE));

    ASSERT_EQ(profileTypes, nymData_.SocialMediaProfileTypes());
}

TEST_F(Test_NymData, Type)
{
    ASSERT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, nymData_.Type());
}

TEST_F(Test_NymData, Valid) { ASSERT_TRUE(nymData_.Valid()); }

TEST_F(Test_NymData, VerificationSet)
{
    // TODO: Add more thorough tests when there are OT classes for the proto
    // verification classes.
    std::unique_ptr<opentxs::proto::VerificationSet> verificationSet =
        nymData_.VerificationSet();
    ASSERT_TRUE(nullptr == verificationSet);
}
