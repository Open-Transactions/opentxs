// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
namespace internal
{
class Purse;
}  // namespace internal

class Mint;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Token;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::internal
{
class Token
{
public:
    virtual auto ID(const PasswordPrompt& reason) const
        -> UnallocatedCString = 0;
    virtual auto IsSpent(const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Notary() const -> const identifier::Notary& = 0;
    virtual auto Owner() const noexcept -> blind::internal::Purse& = 0;
    virtual auto Series() const -> MintSeries = 0;
    virtual auto State() const -> blind::TokenState = 0;
    virtual auto Type() const -> blind::CashType = 0;
    virtual auto Unit() const -> const identifier::UnitDefinition& = 0;
    virtual auto ValidFrom() const -> Time = 0;
    virtual auto ValidTo() const -> Time = 0;
    virtual auto Value() const -> Denomination = 0;
    virtual auto Serialize(proto::Token& out) const noexcept -> bool = 0;

    virtual auto ChangeOwner(
        blind::internal::Purse& oldOwner,
        blind::internal::Purse& newOwner,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto MarkSpent(const PasswordPrompt& reason) -> bool = 0;
    virtual auto Process(
        const identity::Nym& owner,
        const blind::Mint& mint,
        const PasswordPrompt& reason) -> bool = 0;

    virtual ~Token() = default;
};
}  // namespace opentxs::otx::blind::internal
