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

TEST(Contact, Contact)
{
    StorageMock storage;
    opentxs::ClientMock client(storage);

    opentxs::CryptoMock crypto;
    opentxs::EncodeMock encode;
    opentxs::FactoryMock factory;

    ON_CALL(client, Crypto)
        .WillByDefault(::testing::Invoke([&crypto]() -> const opentxs::api::session::Crypto& { return crypto; }));
    ON_CALL(crypto, Encode)
        .WillByDefault(::testing::Invoke([&encode]() -> opentxs::api::crypto::Encode& { return encode; }));
    ON_CALL(encode, Nonce(testing::_,testing::_))
        .WillByDefault(testing::Return(opentxs::String::Factory()));

    opentxs::Contact contact(client, "123");
}