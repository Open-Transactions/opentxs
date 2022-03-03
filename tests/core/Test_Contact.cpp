// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "ClientMock.hpp"
#include "StorageMock.hpp"
#include "CryptoMock.hpp"
#include "EncodeMock.hpp"
#include "FactoryMock.hpp"
#include "opentxs/core/Contact.hpp"

class Test_Contact : public ::testing::Test {
public:
    StorageMock storage_;
    opentxs::CryptoMock crypto_;
    opentxs::EncodeMock encode_;
    opentxs::FactoryMock factory_;
    opentxs::ClientMock client_;

    Test_Contact() : client_(storage_)
    {

    }

    void SetUp() override
    {
        ON_CALL(client_, Crypto)
                .WillByDefault(::testing::Invoke([this]() -> const opentxs::api::session::Crypto& { return crypto_; }));
        ON_CALL(client_, Factory)
                .WillByDefault(::testing::Invoke([this]() -> const opentxs::api::session::Factory& { return factory_; }));
        ON_CALL(crypto_, Encode)
                .WillByDefault(::testing::Invoke([this]() -> opentxs::api::crypto::Encode& { return encode_; }));
        ON_CALL(encode_, Nonce(testing::_,testing::_))
                .WillByDefault(testing::Return(opentxs::String::Factory()));

        ON_CALL(factory_, Identifier())
                .WillByDefault(testing::Return(opentxs::Identifier::Factory()));
//    ON_CALL(factory, Identifier(testing::Matcher<const std::string_view>()))
//            .WillByDefault(testing::Return(opentxs::Identifier::Factory()));
        ON_CALL(factory_, NymID())
                .WillByDefault(testing::Return(opentxs::identifier::Nym::Factory()));
    }
};

TEST_F(Test_Contact, Contact)
{
    opentxs::Contact contact(client_, "test_label");
    const std::string test_email = "test_email@example.com";
    const std::string phone_number = "500600700";

    EXPECT_EQ(contact.EmailAddresses(),"");

    EXPECT_TRUE(contact.AddEmail(test_email, true, false));
    EXPECT_EQ(contact.EmailAddresses(),test_email);

//    EXPECT_TRUE(contact.AddPhoneNumber(phone_number, true, false));
//    EXPECT_EQ(contact.EmailAddresses(),test_email);
}