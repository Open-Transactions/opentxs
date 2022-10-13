// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session
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
namespace internal
{
class Purse;
}  // namespace internal

class Purse;
class Token;
}  // namespace blind
}  // namespace otx

class Amount;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
auto swap(Purse& lhs, Purse& rhs) noexcept -> void;

class OPENTXS_EXPORT Purse
{
public:
    class Imp;

    using iterator = opentxs::iterator::Bidirectional<Purse, Token>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Purse, const Token>;

    operator bool() const noexcept;

    auto at(const std::size_t position) const -> const Token&;
    auto begin() const noexcept -> const_iterator;
    auto cbegin() const noexcept -> const_iterator;
    auto cend() const noexcept -> const_iterator;
    auto EarliestValidTo() const -> Time;
    auto end() const noexcept -> const_iterator;
    auto Internal() const noexcept -> const internal::Purse&;
    auto IsUnlocked() const -> bool;
    auto LatestValidFrom() const -> Time;
    auto Notary() const -> const identifier::Notary&;
    auto Serialize(AllocateOutput destination) const noexcept -> bool;
    auto size() const noexcept -> std::size_t;
    auto State() const -> blind::PurseType;
    auto Type() const -> blind::CashType;
    auto Unit() const -> const identifier::UnitDefinition&;
    auto Unlock(const identity::Nym& nym, const PasswordPrompt& reason) const
        -> bool;
    auto Verify(const api::session::Notary& server) const -> bool;
    auto Value() const -> const Amount&;

    auto AddNym(const identity::Nym& nym, const PasswordPrompt& reason) -> bool;
    auto at(const std::size_t position) -> Token&;
    auto begin() noexcept -> iterator;
    auto end() noexcept -> iterator;
    auto Internal() noexcept -> internal::Purse&;
    auto Pop() -> Token;
    auto Push(Token&& token, const PasswordPrompt& reason) -> bool;
    auto swap(Purse& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Purse(Imp* imp) noexcept;
    Purse() noexcept;
    Purse(const Purse&) noexcept;
    Purse(Purse&&) noexcept;
    auto operator=(const Purse&) noexcept -> Purse&;
    auto operator=(Purse&&) noexcept -> Purse&;

    virtual ~Purse();

private:
    Imp* imp_;
};
}  // namespace opentxs::otx::blind
