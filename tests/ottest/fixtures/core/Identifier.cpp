// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/Identifier.hpp"  // IWYU pragma: associated

#include <Identifier.pb.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/api/FactoryAPI.hpp"
#include "internal/core/identifier/Identifier.hpp"

namespace ottest
{
Identifier::Identifier() noexcept
    : generic_()
    , notary_()
    , nym_()
    , unit_()
{
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

UnitID::UnitID() noexcept
    : id_(unit_)
{
    id_ = ot_.Factory().Internal().UnitIDFromRandom();
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

auto Identifier::RandomUnitID() const noexcept -> ot::identifier::UnitDefinition
{
    return ot_.Factory().Internal().UnitIDFromRandom();
}

auto GenericID::CheckProtobufSerialization(
    const ot::identifier::Generic& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::proto::Identifier{};
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
    auto proto = ot::proto::Identifier{};
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
    auto proto = ot::proto::Identifier{};
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

auto UnitID::CheckProtobufSerialization(
    const ot::identifier::UnitDefinition& in) const noexcept -> bool
{
    auto out{true};
    auto proto = ot::proto::Identifier{};
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
