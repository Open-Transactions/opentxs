// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
namespace internal
{
class Token;
}  // namespace internal

class Token;
}  // namespace blind
}  // namespace otx

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
auto swap(Token& lhs, Token& rhs) noexcept -> void;

class OPENTXS_EXPORT Token
{
public:
    class Imp;

    operator bool() const noexcept;

    auto ID(const PasswordPrompt& reason) const -> UnallocatedCString;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Token&;
    auto IsSpent(const PasswordPrompt& reason) const -> bool;
    auto Notary() const -> const identifier::Notary&;
    auto Series() const -> MintSeries;
    auto State() const -> blind::TokenState;
    auto Type() const -> blind::CashType;
    auto Unit() const -> const identifier::UnitDefinition&;
    auto ValidFrom() const -> Time;
    auto ValidTo() const -> Time;
    auto Value() const -> Denomination;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Token&;
    auto swap(Token&) noexcept -> void;

    OPENTXS_NO_EXPORT Token(Imp* imp) noexcept;
    Token() noexcept;
    Token(const Token&) noexcept;
    Token(Token&&) noexcept;
    auto operator=(const Token&) noexcept -> Token&;
    auto operator=(Token&&) noexcept -> Token&;

    virtual ~Token();

private:
    Imp* imp_;
};
}  // namespace opentxs::otx::blind
