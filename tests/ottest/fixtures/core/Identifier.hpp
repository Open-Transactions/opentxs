// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/LowLevel.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class Identifier;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
namespace ot = opentxs;

OPENTXS_EXPORT auto serialize_identifier_to_pb(
    const opentxs::identifier::Generic& id,
    opentxs::protobuf::Identifier& out) noexcept -> void;

class OPENTXS_EXPORT Identifier : public LowLevel
{
protected:
    ot::identifier::Generic generic_;
    ot::identifier::Notary notary_;
    ot::identifier::Nym nym_;
    ot::identifier::UnitDefinition unit_;
    ot::identifier::Account generic_account_;
    ot::identifier::Account blockchain_account_;
    ot::identifier::Account custodial_account_;
    ot::identifier::HDSeed seed_;

    auto RandomAccountID() const noexcept -> ot::identifier::Account;
    auto RandomBlockchainAccountID() const noexcept -> ot::identifier::Account;
    auto RandomCustodialAccountID() const noexcept -> ot::identifier::Account;
    auto RandomID() const noexcept -> ot::identifier::Generic;
    auto RandomNotaryID() const noexcept -> ot::identifier::Notary;
    auto RandomNymID() const noexcept -> ot::identifier::Nym;
    auto RandomSeedID() const noexcept -> ot::identifier::HDSeed;
    auto RandomUnitID() const noexcept -> ot::identifier::UnitDefinition;

    Identifier() noexcept;

    ~Identifier() override = default;
};

class OPENTXS_EXPORT AccountID : public Identifier
{
protected:
    ot::identifier::Account id_;

    auto CheckProtobufSerialization(
        const ot::identifier::Account& in) const noexcept -> bool;

    AccountID() noexcept;

    ~AccountID() override = default;
};

class OPENTXS_EXPORT GenericID : public Identifier
{
protected:
    ot::identifier::Generic id_;

    auto CheckProtobufSerialization(
        const ot::identifier::Generic& in) const noexcept -> bool;

    GenericID() noexcept;

    ~GenericID() override = default;
};

class OPENTXS_EXPORT NotaryID : public Identifier
{
protected:
    ot::identifier::Notary id_;

    auto CheckProtobufSerialization(
        const ot::identifier::Notary& in) const noexcept -> bool;

    NotaryID() noexcept;

    ~NotaryID() override = default;
};

class OPENTXS_EXPORT NymID : public Identifier
{
protected:
    ot::identifier::Nym id_;

    auto CheckProtobufSerialization(
        const ot::identifier::Nym& in) const noexcept -> bool;

    NymID() noexcept;

    ~NymID() override = default;
};

class OPENTXS_EXPORT SeedID : public Identifier
{
protected:
    ot::identifier::HDSeed id_;

    auto CheckProtobufSerialization(
        const ot::identifier::HDSeed& in) const noexcept -> bool;

    SeedID() noexcept;

    ~SeedID() override = default;
};

class OPENTXS_EXPORT UnitID : public Identifier
{
protected:
    ot::identifier::UnitDefinition id_;

    auto CheckProtobufSerialization(
        const ot::identifier::UnitDefinition& in) const noexcept -> bool;

    UnitID() noexcept;

    ~UnitID() override = default;
};
}  // namespace ottest
