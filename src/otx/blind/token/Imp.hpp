// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "Proto.hpp"
#include "internal/otx/blind/Purse.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/symmetric/Algorithm.hpp"
#include "opentxs/otx/blind/CashType.hpp"
#include "opentxs/otx/blind/Purse.hpp"
#include "opentxs/otx/blind/Token.hpp"
#include "opentxs/otx/blind/TokenState.hpp"
#include "opentxs/otx/blind/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/blind/token/Token.hpp"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/Token.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

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
class Purse;
}  // namespace blind
}  // namespace otx

namespace proto
{
class Ciphertext;
class Token;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

#define OT_TOKEN_VERSION 1

namespace opentxs::otx::blind::token
{
class Token : virtual public blind::Token::Imp
{
public:
    auto IsValid() const noexcept -> bool final { return true; }
    auto Notary() const -> const identifier::Notary& override
    {
        return notary_;
    }
    auto Owner() const noexcept -> blind::internal::Purse& final
    {
        return purse_;
    }
    auto Series() const -> MintSeries override { return series_; }
    auto State() const -> blind::TokenState override { return state_; }
    auto Type() const -> blind::CashType override { return type_; }
    auto Unit() const -> const identifier::UnitDefinition& override
    {
        return unit_;
    }
    auto ValidFrom() const -> Time override { return valid_from_; }
    auto ValidTo() const -> Time override { return valid_to_; }
    auto Value() const -> Denomination override { return denomination_; }

    virtual auto GenerateTokenRequest(
        const identity::Nym& owner,
        const Mint& mint,
        const PasswordPrompt& reason) -> bool = 0;

    ~Token() override = default;

protected:
    static const opentxs::crypto::key::symmetric::Algorithm mode_;

    const api::Session& api_;
    blind::internal::Purse& purse_;
    blind::TokenState state_;
    const OTNotaryID notary_;
    const OTUnitID unit_;
    const std::uint64_t series_;
    const Denomination denomination_;
    const Time valid_from_;
    const Time valid_to_;

    auto reencrypt(
        const crypto::key::Symmetric& oldKey,
        const PasswordPrompt& oldPassword,
        const crypto::key::Symmetric& newKey,
        const PasswordPrompt& newPassword,
        proto::Ciphertext& ciphertext) -> bool;

    auto Serialize(proto::Token& out) const noexcept -> bool override;

    Token(
        const api::Session& api,
        blind::internal::Purse& purse,
        const proto::Token& serialized);
    Token(
        const api::Session& api,
        blind::internal::Purse& purse,
        const VersionNumber version,
        const blind::TokenState state,
        const std::uint64_t series,
        const Denomination denomination,
        const Time validFrom,
        const Time validTo);
    Token(const Token&);

private:
    const blind::CashType type_;
    const VersionNumber version_;

    Token(
        const api::Session& api,
        blind::internal::Purse& purse,
        const blind::TokenState state,
        const blind::CashType type,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const Denomination denomination,
        const Time validFrom,
        const Time validTo,
        const VersionNumber version);
    Token() = delete;
    Token(Token&&) = delete;
    auto operator=(const Token&) -> Token& = delete;
    auto operator=(Token&&) -> Token& = delete;
};
}  // namespace opentxs::otx::blind::token
