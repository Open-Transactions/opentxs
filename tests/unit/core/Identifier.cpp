// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/fixtures/core/Identifier.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;
using enum opentxs::identifier::Type;
using enum opentxs::identifier::AccountSubtype;
using enum opentxs::AccountType;

TEST_F(Identifier, type)
{
    EXPECT_EQ(generic_.Type(), generic);
    EXPECT_EQ(notary_.Type(), notary);
    EXPECT_EQ(nym_.Type(), nym);
    EXPECT_EQ(unit_.Type(), unitdefinition);
    EXPECT_EQ(generic_account_.Type(), account);
    EXPECT_EQ(blockchain_account_.Type(), account);
    EXPECT_EQ(custodial_account_.Type(), account);
    EXPECT_EQ(seed_.Type(), hdseed);
}

TEST_F(Identifier, subtype)
{
    EXPECT_EQ(generic_account_.Subtype(), invalid_subtype);
    EXPECT_EQ(generic_account_.AccountType(), Error);
    EXPECT_EQ(blockchain_account_.Type(), account);
    EXPECT_EQ(blockchain_account_.Subtype(), invalid_subtype);
    EXPECT_EQ(blockchain_account_.AccountType(), Error);
    EXPECT_EQ(custodial_account_.Type(), account);
    EXPECT_EQ(custodial_account_.Subtype(), invalid_subtype);
    EXPECT_EQ(custodial_account_.AccountType(), Error);
}

TEST_F(Identifier, copy_constructor)
{
    generic_ = RandomID();
    notary_ = RandomNotaryID();
    nym_ = RandomNymID();
    unit_ = RandomUnitID();
    generic_account_ = RandomAccountID();
    blockchain_account_ = RandomBlockchainAccountID();
    custodial_account_ = RandomCustodialAccountID();
    seed_ = RandomSeedID();

    EXPECT_EQ(generic_account_.Subtype(), invalid_subtype);
    EXPECT_EQ(generic_account_.AccountType(), Error);
    EXPECT_EQ(blockchain_account_.Subtype(), blockchain_account);
    EXPECT_EQ(blockchain_account_.AccountType(), Blockchain);
    EXPECT_EQ(custodial_account_.Subtype(), custodial_account);
    EXPECT_EQ(custodial_account_.AccountType(), Custodial);

    {
        const auto& id = generic_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
    }

    {
        const auto& id = notary_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
    }

    {
        const auto& id = nym_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
    }

    {
        const auto& id = unit_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
    }

    {
        const auto& id = generic_account_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
        EXPECT_EQ(copy.Subtype(), id.Subtype());
    }

    {
        const auto& id = blockchain_account_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
        EXPECT_EQ(copy.Subtype(), id.Subtype());
    }

    {
        const auto& id = custodial_account_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
        EXPECT_EQ(copy.Subtype(), id.Subtype());
    }

    {
        const auto& id = seed_;
        auto copy{id};

        EXPECT_EQ(copy, id);
        EXPECT_EQ(print(copy.Type()), print(id.Type()));
        EXPECT_EQ(print(copy.Algorithm()), print(id.Algorithm()));
        EXPECT_EQ(copy.Bytes(), id.Bytes());
    }
}

TEST_F(Identifier, generic_default_accessors)
{
    const auto& id = generic_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, notary_default_accessors)
{
    const auto& id = notary_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, nym_default_accessors)
{
    const auto& id = nym_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, unit_default_accessors)
{
    const auto& id = unit_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, account_default_accessors)
{
    const auto& id = generic_account_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, seed_default_accessors)
{
    const auto& id = seed_;

    EXPECT_EQ(id.data(), nullptr);
    EXPECT_EQ(id.size(), 0_uz);
}

TEST_F(Identifier, generic_serialize_base58_empty)
{
    const auto& id = generic_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().IdentifierFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, notary_serialize_base58_empty)
{
    const auto& id = generic_;
    const auto base58 = notary_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NotaryIDFromBase58(base58);

    EXPECT_EQ(notary_, recovered);
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, nym_serialize_base58_empty)
{
    const auto& id = nym_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NymIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, unit_serialize_base58_empty)
{
    const auto& id = unit_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().UnitIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, account_serialize_base58_empty)
{
    const auto& id = generic_account_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().AccountIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.Subtype(), recovered.Subtype());
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, blockchain_account_serialize_base58_empty)
{
    const auto& id = blockchain_account_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().AccountIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.Subtype(), recovered.Subtype());
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, custodial_account_serialize_base58_empty)
{
    const auto& id = custodial_account_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().AccountIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.Subtype(), recovered.Subtype());
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(Identifier, seed_serialize_base58_empty)
{
    const auto& id = seed_;
    const auto base58 = id.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NymIDFromBase58(base58);

    EXPECT_EQ(id, recovered);
    EXPECT_EQ(id.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(GenericID, generic_serialize_base58_non_empty)
{
    const auto base58 = id_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().IdentifierFromBase58(base58);

    EXPECT_EQ(id_, recovered);
    EXPECT_EQ(id_.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(NotaryID, notary_serialize_base58_non_empty)
{
    const auto base58 = id_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NotaryIDFromBase58(base58);

    EXPECT_EQ(id_, recovered);
    EXPECT_EQ(id_.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(NymID, nym_serialize_base58_non_empty)
{
    const auto base58 = id_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NymIDFromBase58(base58);

    EXPECT_EQ(id_, recovered);
    EXPECT_EQ(id_.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(UnitID, unit_serialize_base58_non_empty)
{
    const auto base58 = id_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().UnitIDFromBase58(base58);

    EXPECT_EQ(id_, recovered);
    EXPECT_EQ(id_.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}

TEST_F(GenericID, generic_serialize_protobuf_non_empty)
{
    EXPECT_TRUE(CheckProtobufSerialization(id_));
}

TEST_F(NotaryID, notary_serialize_protobuf_non_empty)
{
    EXPECT_TRUE(CheckProtobufSerialization(id_));
}

TEST_F(NymID, nym_serialize_protobuf_non_empty)
{
    EXPECT_TRUE(CheckProtobufSerialization(id_));
}

TEST_F(UnitID, unit_serialize_protobuf_non_empty)
{
    EXPECT_TRUE(CheckProtobufSerialization(id_));
}

TEST_F(AccountID, Account_serialize_protobuf_non_empty)
{
    EXPECT_TRUE(CheckProtobufSerialization(id_));
}

TEST_F(SeedID, seed_serialize_base58_non_empty)
{
    const auto base58 = id_.asBase58(ot_.Crypto());
    const auto recovered = ot_.Factory().NymIDFromBase58(base58);

    EXPECT_EQ(id_, recovered);
    EXPECT_EQ(id_.asBase58(ot_.Crypto()), recovered.asBase58(ot_.Crypto()));
}
}  // namespace ottest
