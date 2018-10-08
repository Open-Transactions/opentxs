// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_MINT_HPP
#define OPENTXS_CASH_MINT_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH
#include "opentxs/core/Contract.hpp"

#include <cstdint>
#include <ctime>
#include <map>

namespace opentxs
{
typedef std::map<std::int64_t, OTArmored> mapOfArmor;

class Mint : public Contract
{
public:
    inline std::int32_t GetSeries() const
    {
        return m_nSeries;
    }  // The series ID
    inline time64_t GetValidFrom() const
    {
        return m_VALID_FROM;
    }  // The token "valid from" date for this series
    inline time64_t GetValidTo() const
    {
        return m_VALID_TO;
    }  // The token "valid until" date for this series
    inline time64_t GetExpiration() const
    {
        return m_EXPIRATION;
    }                      // The date the mint expires (should be halfway
                           // between the above two dates.)
    bool Expired() const;  // true or false. Expired?
    // Valid range is GetValidFrom() through GetExpiration().
    // Server-side only.
    inline OTIdentifier AccountID() const { return m_CashAccountID; }

    void Release() override;
    void Release_Mint();
    void ReleaseDenominations();
    bool LoadMint(const char* szAppend = nullptr);
    bool SaveMint(const char* szAppend = nullptr);

    bool LoadContract() override;

    // Will save the private keys on next serialization (not just public keys)
    // (SignContract sets m_bSavePrivateKeys back to false again.)
    inline void SetSavePrivateKeys(bool bDoIt = true)
    {
        m_bSavePrivateKeys = bDoIt;
    }

    // The denomination indicated here is the actual denomination...1, 5, 20,
    // 50, 100, etc
    bool GetPrivate(Armored& theArmor, std::int64_t lDenomination);
    bool GetPublic(Armored& theArmor, std::int64_t lDenomination);

    std::int64_t GetDenomination(std::int32_t nIndex);
    std::int64_t GetLargestDenomination(std::int64_t lAmount);
    virtual bool AddDenomination(
        const Nym& theNotary,
        std::int64_t lDenomination,
        std::int32_t nPrimeLength = 1024) = 0;

    inline std::int32_t GetDenominationCount() const
    {
        return m_nDenominationCount;
    }

    bool VerifyContractID() const override;

    bool VerifyMint(const Nym& theOperator);

    void UpdateContents() override;  // Before transmission or serialization,
                                     // this
                                     // is where the token saves its contents

    inline void SetInstrumentDefinitionID(const Identifier& newID)
    {
        m_InstrumentDefinitionID = newID;
    }

    // Lucre step 1: generate new mint
    void GenerateNewMint(
        const api::Wallet& wallet,
        std::int32_t nSeries,
        time64_t VALID_FROM,
        time64_t VALID_TO,
        time64_t MINT_EXPIRATION,
        const Identifier& theInstrumentDefinitionID,
        const Identifier& theNotaryID,
        const Nym& theNotary,
        std::int64_t nDenom1 = 0,
        std::int64_t nDenom2 = 0,
        std::int64_t nDenom3 = 0,
        std::int64_t nDenom4 = 0,
        std::int64_t nDenom5 = 0,
        std::int64_t nDenom6 = 0,
        std::int64_t nDenom7 = 0,
        std::int64_t nDenom8 = 0,
        std::int64_t nDenom9 = 0,
        std::int64_t nDenom10 = 0);

    // step 2: (coin request is in Token)

    // Lucre step 3: mint signs token
    virtual bool SignToken(
        const Nym& theNotary,
        Token& theToken,
        String& theOutput,
        std::int32_t nTokenIndex) = 0;

    // step 4: (unblind coin is in Token)

    // Lucre step 5: mint verifies token when it is redeemed by merchant.
    virtual bool VerifyToken(
        const Nym& theNotary,
        String& theCleartextToken,
        std::int64_t lDenomination) = 0;

    virtual ~Mint();

protected:
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void InitMint();

    mapOfArmor m_mapPrivate;  // An ENVELOPE. You need to pass the Pseudonym to
                              // every method that uses this. Private.
    // Then you have to set it into an envelope and then open it using the Nym.
    // Encrypted.
    mapOfArmor m_mapPublic;  // An Ascii-armored string of the mint Public
                             // information. Base64-encoded only.

    // The Notary ID, (a hash of the server contract whose public key is
    // m_keyPublic)
    OTIdentifier m_NotaryID;
    // The Nym ID of the Server, whose public key is m_keyPublic
    OTIdentifier m_ServerNymID;
    // Each Asset type has its own mint.
    OTIdentifier m_InstrumentDefinitionID;

    std::int32_t m_nDenominationCount{0};  // How many denominations of the
                                           // currency are issued by this Mint?
                                           // (Each requires its own key pair.)

    bool m_bSavePrivateKeys{false};  // Determines whether it serializes private
                                     // keys (no if false).
    // Set this to 'true' before signing, if you want the private keys included.
    // (The signing process will then automatically set it back to false again.)

    // --- MINT SERIES with EXPIRATION DATES ------------------
    std::int32_t m_nSeries{0};  // Each series of the mint has a valid from and
                                // to date. Series should rotate.
    // (That is, the new one should be introduced halfway through the validity
    // period
    // of the current one, and so on...)

    time64_t m_VALID_FROM{0};  // All tokens generated by this series will have
                               // the same From and To dates.
    time64_t m_VALID_TO{0};    // This way they cannot be tracked by expiration
                               // date.

    time64_t m_EXPIRATION{0};  // The Mint itself expires before the tokens do.
                               // (As the new series rotates in...)

    OTIdentifier m_CashAccountID;  // The Account ID for the cash reserve
                                   // account.

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

private:  // Private prevents erroneous use by other classes.
    typedef Contract ot_super;

    Mint() = delete;
};
}  // namespace opentxs
#endif  // OT_CASH
#endif
