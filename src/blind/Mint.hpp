// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/blind/Mint.hpp"

#include <map>

namespace opentxs::blind::mint::implementation
{
class Mint : virtual public opentxs::blind::Mint, virtual public Contract
{
public:
    OTIdentifier AccountID() const override { return m_CashAccountID; }
    bool Expired() const override;
    std::int64_t GetDenomination(std::int32_t nIndex) const override;
    std::int32_t GetDenominationCount() const override
    {
        return m_nDenominationCount;
    }

    time64_t GetExpiration() const override { return m_EXPIRATION; }
    std::int64_t GetLargestDenomination(std::int64_t lAmount) const override;
    bool GetPrivate(Armored& theArmor, std::int64_t lDenomination)
        const override;
    bool GetPublic(Armored& theArmor, std::int64_t lDenomination)
        const override;
    std::int32_t GetSeries() const override { return m_nSeries; }
    time64_t GetValidFrom() const override { return m_VALID_FROM; }
    time64_t GetValidTo() const override { return m_VALID_TO; }
    const Identifier& InstrumentDefinitionID() const override
    {
        return m_InstrumentDefinitionID;
    }

    void GenerateNewMint(
        const api::Wallet& wallet,
        std::int32_t nSeries,
        time64_t VALID_FROM,
        time64_t VALID_TO,
        time64_t MINT_EXPIRATION,
        const Identifier& theInstrumentDefinitionID,
        const Identifier& theNotaryID,
        const Nym& theNotary,
        const std::int64_t nDenom1,
        const std::int64_t nDenom2,
        const std::int64_t nDenom3,
        const std::int64_t nDenom4,
        const std::int64_t nDenom5,
        const std::int64_t nDenom6,
        const std::int64_t nDenom7,
        const std::int64_t nDenom8,
        const std::int64_t nDenom9,
        const std::int64_t nDenom10,
        const std::size_t keySize) override;
    bool LoadContract() override;
    bool LoadMint(const char* szAppend = nullptr) override;
    void Release() override;
    void Release_Mint() override;
    void ReleaseDenominations() override;
    bool SaveMint(const char* szAppend = nullptr) override;
    void SetInstrumentDefinitionID(const Identifier& newID) override
    {
        m_InstrumentDefinitionID = newID;
    }
    void SetSavePrivateKeys(bool bDoIt = true) override
    {
        m_bSavePrivateKeys = bDoIt;
    }
    void UpdateContents() override;
    bool VerifyContractID() const override;
    bool VerifyMint(const Nym& theOperator) override;

    ~Mint() override;

protected:
    using mapOfArmor = std::map<std::int64_t, OTArmored>;

    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void InitMint();

    mapOfArmor m_mapPrivate;
    mapOfArmor m_mapPublic;
    OTIdentifier m_NotaryID;
    OTNymID m_ServerNymID;
    OTIdentifier m_InstrumentDefinitionID;
    std::int32_t m_nDenominationCount{0};
    bool m_bSavePrivateKeys{false};
    std::int32_t m_nSeries{0};
    time64_t m_VALID_FROM{0};
    time64_t m_VALID_TO{0};
    time64_t m_EXPIRATION{0};
    OTIdentifier m_CashAccountID;

    Mint(const api::Core& core);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);

private:
    Mint() = delete;
};
}  // namespace opentxs::blind::mint::implementation
