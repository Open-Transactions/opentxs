// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/Identifier.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/Identifier.pb.h>

#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/Factory.internal.hpp"

namespace ottest
{
auto serialize_identifier_to_pb(
    const opentxs::identifier::Generic& id,
    opentxs::protobuf::Identifier& out) noexcept -> void
{
    id.Internal().Serialize(out);
}

Identifier::Identifier() noexcept
    : generic_()
    , notary_()
    , nym_()
    , unit_()
    , generic_account_()
    , blockchain_account_()
    , custodial_account_()
    , seed_()
{
}

AccountID::AccountID() noexcept
    : id_(blockchain_account_)
{
    id_ = ot_.Factory().Internal().AccountIDFromRandom(
        ot::identifier::AccountSubtype::blockchain_account);
}

GenericID::GenericID() noexcept
    : id_(generic_)
{
    id_ = ot_.Factory().Internal().IdentifierFromRandom();
}

NotaryID::NotaryID() noexcept
    : id_(notary_)
{
    id_ = ot_.Factory().Internal().NotaryIDFromRandom();
}

NymID::NymID() noexcept
    : id_(nym_)
{
    id_ = ot_.Factory().Internal().NymIDFromRandom();
}

SeedID::SeedID() noexcept
    : id_(seed_)
{
    id_ = ot_.Factory().Internal().SeedIDFromRandom();
}

UnitID::UnitID() noexcept
    : id_(unit_)
{
    id_ = ot_.Factory().Internal().UnitIDFromRandom();
}

auto Identifier::RandomAccountID() const noexcept -> ot::identifier::Account
{
    return ot_.Factory().Internal().AccountIDFromRandom(
        ot::identifier::AccountSubtype::invalid_subtype);
}

auto Identifier::RandomBlockchainAccountID() const noexcept
    -> ot::identifier::Account
{
    return ot_.Factory().Internal().AccountIDFromRandom(
        ot::identifier::AccountSubtype::blockchain_account);
}

auto Identifier::RandomCustodialAccountID() const noexcept
    -> ot::identifier::Account
{
    return ot_.Factory().Internal().AccountIDFromRandom(
        ot::identifier::AccountSubtype::custodial_account);
}

auto Identifier::RandomID() const noexcept -> ot::identifier::Generic
{
    return ot_.Factory().Internal().IdentifierFromRandom();
}

auto Identifier::RandomNotaryID() const noexcept -> ot::identifier::Notary
{
    return ot_.Factory().Internal().NotaryIDFromRandom();
}

auto Identifier::RandomNymID() const noexcept -> ot::identifier::Nym
{
    return ot_.Factory().Internal().NymIDFromRandom();
}

auto Identifier::RandomSeedID() const noexcept -> ot::identifier::HDSeed
{
    return ot_.Factory().Internal().SeedIDFromRandom();
}

auto Identifier::RandomUnitID() const noexcept -> ot::identifier::UnitDefinition
{
    return ot_.Factory().Internal().UnitIDFromRandom();
}

auto AccountID::CheckProtobufSerialization(
    const ot::identifier::Account& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().Identifier(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}

auto GenericID::CheckProtobufSerialization(
    const ot::identifier::Generic& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().Identifier(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}

auto NotaryID::CheckProtobufSerialization(
    const ot::identifier::Notary& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().NotaryID(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}

auto NymID::CheckProtobufSerialization(
    const ot::identifier::Nym& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().NymID(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}

auto SeedID::CheckProtobufSerialization(
    const ot::identifier::HDSeed& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().SeedID(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}

auto UnitID::CheckProtobufSerialization(
    const ot::identifier::UnitDefinition& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::protobuf::Identifier{};
    const auto serialized = in.Internal().Serialize(proto);
    const auto recovered = ot_.Factory().Internal().UnitID(proto);
    const auto expectedBase58 = in.asBase58(ot_.Crypto());
    const auto recoveredBase58 = recovered.asBase58(ot_.Crypto());
    out &= serialized;
    out &= (in == recovered);
    out &= (expectedBase58 == recoveredBase58);

    EXPECT_TRUE(serialized);
    EXPECT_EQ(in, recovered);
    EXPECT_EQ(expectedBase58, recoveredBase58);

    return out;
}
}  // namespace ottest
