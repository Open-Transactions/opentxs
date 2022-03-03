// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "internal/otx/blind/Mint.hpp"
#include "internal/otx/common/Contract.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/blind/Mint.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "otx/blind/mint/Mint.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class Notary;
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class PasswordPrompt;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind::mint
{
class Mint : public blind::Mint::Imp
{
public:
    auto AccountID() const -> OTIdentifier override { return m_CashAccountID; }
    auto Expired() const -> bool override;
    auto GetDenomination(std::int32_t nIndex) const -> Amount override;
    auto GetDenominationCount() const -> std::int32_t override
    {
        return m_nDenominationCount;
    }

    auto GetExpiration() const -> Time override { return m_EXPIRATION; }
    auto GetLargestDenomination(const Amount& lAmount) const -> Amount override;
    auto GetPrivate(Armored& theArmor, const Amount& lDenomination) const
        -> bool override;
    auto GetPublic(Armored& theArmor, const Amount& lDenomination) const
        -> bool override;
    auto GetSeries() const -> std::int32_t override { return m_nSeries; }
    auto GetValidFrom() const -> Time override { return m_VALID_FROM; }
    auto GetValidTo() const -> Time override { return m_VALID_TO; }
    auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& override
    {
        return m_InstrumentDefinitionID;
    }
    auto isValid() const noexcept -> bool final { return true; }

    void GenerateNewMint(
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
        const PasswordPrompt& reason) override;
    auto LoadContract() -> bool override;
    auto LoadMint(const char* szAppend = nullptr) -> bool override;
    void Release() override;
    void Release_Mint() override;
    void ReleaseDenominations() override;
    auto SaveMint(const char* szAppend = nullptr) -> bool override;
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) override
    {
        m_InstrumentDefinitionID = newID;
    }
    void SetSavePrivateKeys(bool bDoIt = true) override
    {
        m_bSavePrivateKeys = bDoIt;
    }
    void UpdateContents(const PasswordPrompt& reason) override;
    auto VerifyContractID() const -> bool override;
    auto VerifyMint(const identity::Nym& theOperator) -> bool override;

    ~Mint() override;

protected:
    using mapOfArmor = UnallocatedMap<Amount, OTArmored>;

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void InitMint();

    mapOfArmor m_mapPrivate;
    mapOfArmor m_mapPublic;
    OTNotaryID m_NotaryID;
    OTNymID m_ServerNymID;
    OTUnitID m_InstrumentDefinitionID;
    std::int32_t m_nDenominationCount;
    bool m_bSavePrivateKeys;
    std::int32_t m_nSeries;
    Time m_VALID_FROM;
    Time m_VALID_TO;
    Time m_EXPIRATION;
    OTIdentifier m_CashAccountID;

    Mint(const api::Session& api);
    Mint(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::UnitDefinition& unit);
    Mint(
        const api::Session& api,
        const identifier::Notary& notary,
        const identifier::Nym& serverNym,
        const identifier::UnitDefinition& unit);

private:
    Mint() = delete;
};
}  // namespace opentxs::otx::blind::mint
