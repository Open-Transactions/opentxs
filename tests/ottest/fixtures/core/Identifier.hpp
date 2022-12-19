// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/LowLevel.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Identifier : public LowLevel
{
protected:
    ot::identifier::Generic generic_;
    ot::identifier::Notary notary_;
    ot::identifier::Nym nym_;
    ot::identifier::UnitDefinition unit_;

    auto RandomID() const noexcept -> ot::identifier::Generic;
    auto RandomNotaryID() const noexcept -> ot::identifier::Notary;
    auto RandomNymID() const noexcept -> ot::identifier::Nym;
    auto RandomUnitID() const noexcept -> ot::identifier::UnitDefinition;

    Identifier() noexcept;

    ~Identifier() override = default;
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
