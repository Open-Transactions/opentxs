// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <ctime>
#include <string_view>

#include "internal/otx/common/Contract.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Wallet;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace blind
{
class Token;
}  // namespace blind
}  // namespace otx

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::internal
{
class Mint : public Contract
{
public:
    virtual auto AccountID() const -> identifier::Account = 0;
    auto API() const noexcept -> const api::Session& { return api_; }
    virtual auto Expired() const -> bool = 0;
    virtual auto GetDenomination(std::int32_t nIndex) const -> Amount = 0;
    virtual auto GetDenominationCount() const -> std::int32_t = 0;
    virtual auto GetExpiration() const -> Time = 0;
    virtual auto GetLargestDenomination(const Amount& lAmount) const
        -> Amount = 0;
    virtual auto GetPrivate(Armored& theArmor, const Amount& lDenomination)
        const -> bool = 0;
    virtual auto GetPublic(Armored& theArmor, const Amount& lDenomination) const
        -> bool = 0;
    virtual auto GetSeries() const -> std::int32_t = 0;
    virtual auto GetValidFrom() const -> Time = 0;
    virtual auto GetValidTo() const -> Time = 0;
    virtual auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& = 0;
    virtual auto AddDenomination(
        const identity::Nym& theNotary,
        const Amount& denomination,
        const std::size_t keySize,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto GenerateNewMint(
        const api::session::Wallet& wallet,
        const std::int32_t nSeries,
        const Time VALID_FROM,
        const Time VALID_TO,
        const Time MINT_EXPIRATION,
        const identifier::UnitDefinition& theInstrumentDefinitionID,
        const identifier::Notary& theNotaryID,
        const identity::Nym& theNotary,
        const Amount& nDenom1,
        const Amount& nDenom2,
        const Amount& nDenom3,
        const Amount& nDenom4,
        const Amount& nDenom5,
        const Amount& nDenom6,
        const Amount& nDenom7,
        const Amount& nDenom8,
        const Amount& nDenom9,
        const Amount& nDenom10,
        const std::size_t keySize,
        const PasswordPrompt& reason) -> void = 0;
    virtual auto LoadMint(std::string_view extension) -> bool = 0;
    virtual auto Release_Mint() -> void = 0;
    virtual auto ReleaseDenominations() -> void = 0;
    virtual auto SaveMint(std::string_view extension) -> bool = 0;
    virtual auto SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) -> void = 0;
    virtual auto SetSavePrivateKeys(bool bDoIt = true) -> void = 0;
    virtual auto SignToken(
        const identity::Nym& notary,
        opentxs::otx::blind::Token& token,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto VerifyMint(const identity::Nym& theOperator) -> bool = 0;
    virtual auto VerifyToken(
        const identity::Nym& notary,
        const opentxs::otx::blind::Token& token,
        const PasswordPrompt& reason) -> bool = 0;

    Mint(const api::Session& api) noexcept;
    Mint(
        const api::Session& api,
        const identifier::UnitDefinition& unit) noexcept;

    Mint() = delete;

    ~Mint() override = default;
};
}  // namespace opentxs::otx::blind::internal
