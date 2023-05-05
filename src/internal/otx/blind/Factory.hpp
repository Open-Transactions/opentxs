// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Types.hpp"

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
class Nym;
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
class Token;
class Purse;
}  // namespace blind

namespace context
{
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class Purse;
class Token;
}  // namespace proto

class Amount;
class PasswordPrompt;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto MintLucre(const api::Session& api) noexcept -> otx::blind::Mint;
auto MintLucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit) noexcept -> otx::blind::Mint;
auto MintLucre(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit) noexcept -> otx::blind::Mint;
auto Purse(const api::Session& api, const proto::Purse& serialized) noexcept
    -> otx::blind::Purse;
auto Purse(const api::Session& api, const ReadView& serialized) noexcept
    -> otx::blind::Purse;
auto Purse(
    const api::Session& api,
    const otx::context::Server&,
    const otx::blind::CashType type,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Purse;
auto Purse(
    const api::Session& api,
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identity::Nym& serverNym,
    const otx::blind::CashType type,
    const otx::blind::Mint& mint,
    const opentxs::Amount& totalValue,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Purse;
auto Purse(
    const api::Session& api,
    const otx::blind::Purse& request,
    const identity::Nym& requester,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Purse;
auto Purse(
    const api::Session& api,
    const identity::Nym& owner,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit,
    const otx::blind::CashType type,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Purse;
auto Token(
    const otx::blind::Token& token,
    otx::blind::internal::Purse& purse) noexcept -> otx::blind::Token;
auto Token(
    const api::Session& api,
    otx::blind::internal::Purse& purse,
    const proto::Token& serialized) noexcept -> otx::blind::Token;
auto Token(
    const api::Session& api,
    const identity::Nym& owner,
    const otx::blind::Mint& mint,
    const otx::blind::Denomination value,
    otx::blind::internal::Purse& purse,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Token;
auto TokenLucre(
    const otx::blind::Token& token,
    otx::blind::internal::Purse& purse) noexcept -> otx::blind::Token;
auto TokenLucre(
    const api::Session& api,
    otx::blind::internal::Purse& purse,
    const proto::Token& serialized) noexcept -> otx::blind::Token;
auto TokenLucre(
    const api::Session& api,
    const identity::Nym& owner,
    const otx::blind::Mint& mint,
    const otx::blind::Denomination value,
    otx::blind::internal::Purse& purse,
    const opentxs::PasswordPrompt& reason) noexcept -> otx::blind::Token;
}  // namespace opentxs::factory
