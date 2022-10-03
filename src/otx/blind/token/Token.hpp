// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/otx/blind/Purse.hpp"
#include "internal/otx/blind/Token.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Token;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
class Token::Imp : virtual public internal::Token
{
public:
    virtual auto clone() const noexcept -> Imp*
    {
        return std::make_unique<Imp>(*this).release();
    }
    auto ID(const PasswordPrompt&) const -> UnallocatedCString override
    {
        return {};
    }
    auto IsSpent(const PasswordPrompt&) const -> bool override { return {}; }
    virtual auto IsValid() const noexcept -> bool { return false; }
    auto Notary() const -> const identifier::Notary& override;
    auto Owner() const noexcept -> internal::Purse& override;
    auto Series() const -> MintSeries override { return {}; }
    auto State() const -> blind::TokenState override { return {}; }
    auto Type() const -> blind::CashType override { return {}; }
    auto Unit() const -> const identifier::UnitDefinition& override;
    auto ValidFrom() const -> Time override { return {}; }
    auto ValidTo() const -> Time override { return {}; }
    auto Value() const -> Denomination override { return {}; }
    auto Serialize(proto::Token&) const noexcept -> bool override { return {}; }

    auto ChangeOwner(
        blind::internal::Purse&,
        blind::internal::Purse&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }
    auto MarkSpent(const PasswordPrompt&) -> bool override { return {}; }
    auto Process(
        const identity::Nym&,
        const blind::Mint&,
        const PasswordPrompt&) -> bool override
    {
        return {};
    }

    Imp() noexcept = default;
    Imp(const Imp&) noexcept {}
    Imp(Imp&&) noexcept {}

    ~Imp() override = default;
};
}  // namespace opentxs::otx::blind
