// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CASH_TOKEN_HPP
#define OPENTXS_CASH_TOKEN_HPP

#include "opentxs/Forward.hpp"

#if OT_CASH

#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Instrument.hpp"

#include <cstdint>
#include <map>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

typedef std::map<std::int32_t, OTArmored> mapOfPrototokens;

/*
Here's a rough sketch of the protocol:

Client requests Mint for withdrawal of 100 ithica work hours.

1) Client blinds and sends N tokens to the server, each worth 100 hours. Client
retains the keys.
2) Server responds with a single index, the one the server has chosen for
signing.
3) Client replies with 99 keys.
4) Server unblinds 99 tokens (or some randomly-chosen % of those) and verifies
them. He signs the last one and returns it.
5) Client receives signed token, unblinds it, stores it for later.
6) When token is redeemed, it has already been unblinded. So Server simply
verifies it.

LAST NAGGING QUESTION: Should the server sign the other 99 tokens before
unblinding them and verifying? In fact, what is it verifying at all?? Certainly
not the amount, which is not even in the Lucre token. If all it does is verify
its signature, then why sign it just to verify it?  Why exactly am I sending 99
tokens? What is the server unblinding them to look for??  Just to make sure all
the IDs are random? That they aren't spent already? I think that's it.  The
client has assurance he chose his own random IDs, the server verifies they are
random and not spent already, and the ID portion is the only part that has to be
randomized.

UPDATE: Ben Laurie has confirmed that the Chaumian 99 token requirement does not
exist with Lucre. All I have to do is send a single blinded token. The server
signs it and sends it back, and the client unblinds it. Only the ID itself is
blinded -- the server can clearly see the amount and only the Mint key for that
denomination will work. */

/** This class implements the Lucre coins. */
class Token : public Instrument
{
public:
    enum tokenState {
        blankToken,
        protoToken,
        signedToken,
        spendableToken,
        verifiedToken,
        errorToken
    };
    // Wallet must submit at least N prototokens per withdrawal request, for the
    // server to notarize it. One server might require at least 5 prototokens
    // per withdrawal. Another might require 100 because it needs more security.
    // Another 1000.  These provide more security but they also cost more in
    // terms of resources to process all those prototokens.
    EXPORT static std::int32_t GetMinimumPrototokenCount();
    EXPORT virtual ~Token();

    EXPORT void Release_Token();
    void Release() override;
    EXPORT void ReleasePrototokens();

    /** Before transmission or serialization, this is where the token saves its
     * contents */
    void UpdateContents() override;
    /** Will save the private keys on next serialization (not just public keys)
     * (SignContract sets m_bSavePrivateKeys back to false again.) */
    inline void SetSavePrivateKeys() { m_bSavePrivateKeys = true; }

    /** When you send a token to the server, you must decrypt it from your own
     * key, and re-encrypt it to the server key, before sending. Use this
     * function to do so. In the case of exporting a token from a purse, you
     * could create a dummy Nym, embed it inside the purse, and then reassign
     * ownership of each token to that token as you push them into that purse.
     * From there, you can hand someone the purse, and password-protect it, if
     * you like. */
    EXPORT bool ReassignOwnership(
        OTNym_or_SymmetricKey& oldOwner,
        OTNym_or_SymmetricKey& newOwner);

    inline const Armored& GetSpendable() const { return m_ascSpendable; }
    inline void SetSpendable(const Armored& theArmor)
    {
        m_ascSpendable->Set(theArmor);
    }
    // TODO potentially return OTPassword here instead OTString (more secure.)
    EXPORT bool GetSpendableString(
        OTNym_or_SymmetricKey theOwner,
        String& theString) const;

    inline Token::tokenState GetState() const { return m_State; }

    // Lucre step 1 (in OTMint) Generate New Mint

    /** Lucre Step 3: Mint signs token (in OTMint) */
    inline std::int32_t GetSeries() const { return m_nSeries; }
    /** (Called by the mint when signing.) */
    inline void SetSeriesAndExpiration(
        std::int32_t nSeries,
        time64_t VALID_FROM,
        time64_t VALID_TO)
    {
        m_nSeries = nSeries;
        m_VALID_FROM = VALID_FROM;
        m_VALID_TO = VALID_TO;
    }

    /** Lucre step 4: client unblinds token -- now it's ready for use. */
    EXPORT virtual bool ProcessToken(
        const Nym& theNym,
        Mint& theMint,
        Token& theRequest) = 0;
    /** Lucre step 5: token verifies when it is redeemed by merchant. Now
     * including spent token database! */
    EXPORT bool VerifyToken(Nym& theNotary, Mint& theMint);
    /** Spent Token Database */
    EXPORT bool IsTokenAlreadySpent(String& theCleartextToken);
    /** Spent Token Database */
    EXPORT bool RecordTokenAsSpent(String& theCleartextToken);
    EXPORT void SetSignature(
        const Armored& theSignature,
        std::int32_t nTokenIndex);
    EXPORT bool GetSignature(Armored& theSignature) const;
    /** The actual denomination of the token is determined by whether or not it
     * verifies when the server uses the private verify info for THAT
     * denomination. So if you set the denomination here wrong, all that does is
     * cause the server to try to verify it with the wrong key. If the
     * proto-token was generated for a different denomination, then it cannot
     * verify. So this value is only here to help you make sure to ask the Mint
     * to use the right key when verifying the token. And this only works
     * because we have a specific set of denominations for each digital asset,
     * each with its own key pair in the Mint.*/
    inline std::int64_t GetDenomination() const { return m_lDenomination; }
    inline void SetDenomination(std::int64_t lVal) { m_lDenomination = lVal; }

    // These are not actually necessary for Lucre itself, which only needs to
    // send a single blinded proto-token. Index is always 0, and Count is always
    // 1. But this does mean OTToken supports digital cash schemes that involve
    // multiple prototokens -- even though Lucre is not one of those.
    EXPORT bool GetPrototoken(Armored& ascPrototoken, std::int32_t nTokenIndex);
    EXPORT bool GetPrivatePrototoken(
        Armored& ascPrototoken,
        std::int32_t nTokenIndex);

protected:
    friend api::implementation::Factory;

    bool m_bPasswordProtected{false};  // this token might be encrypted to a
                                       // passphrase, instead of a Nym.

    OTArmored m_ascSpendable;  // This is the final, signed, unblinded token
                             // ID, ready to be spent. (But still in
                             // envelope form, encrypted and
                             // ascii-armored.)
    OTArmored m_Signature;     // This is the Mint's signature on the blinded
                             // prototoken.

    std::int64_t m_lDenomination{0};  // The actual value of the token is
                                      // between issuer and trader.
    // The token must have a denomination so we know which Mint Key to verify it
    // with.

    // --------------- Prototoken stuff below here.....
    mapOfPrototokens m_mapPublic;   // in protoToken state, this object stores N
                                    // prototokens in order to fulfill the
                                    // protocol
    mapOfPrototokens m_mapPrivate;  // The elements are accessed [0..N].
                                    // mapPublic[2] corresponds to
                                    // map_Private[2], etc.

    std::int32_t m_nTokenCount{0};  // Official token count is stored here for
    // serialization, etc. The maps' size should match.
    std::int32_t m_nChosenIndex{0};  // When the client submits N prototokens,
                                     // the server randomly chooses one to sign.
    // (The server opens the other (N-1) prototokens to verify the amount is
    // correct and that the IDs are random enough.) Expiration dates are
    // necessary because otherwise the spent token database must be stored
    // forever. This may be useful in some applications, but in most, a 1-year
    // or 1-month expiration date will be perfectly fine, especially with
    // auto-exchanges performed by the wallet. Suddenly it becomes much more
    // feasible a proposition to effectively run a token server, without having
    // to hold those spent tokens forever.
    //
    // EXPIRATION DATES are in the parent:
    //
    // time64_t            m_VALID_FROM;    // (In the parent)
    // time64_t            m_VALID_TO;        // (In the parent)
    //
    // Tokens (and Mints) also have a SERIES:
    std::int32_t m_nSeries{0};
    tokenState m_State{errorToken};
    bool m_bSavePrivateKeys{false};  // Determines whether it serializes private
                                     // keys 1 time (yes if true)
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    void InitToken();
    bool ChooseIndex(std::int32_t nIndex);

    /** Lucre Step 2: Generate Coin Request nDenomination MUST be one that the
     * Mint supports. let nTokenCount default to 1, since that's how Lucre
     * works. */
    virtual bool GenerateTokenRequest(
        const Nym& theNym,
        Mint& theMint,
        std::int64_t lDenomination,
        std::int32_t nTokenCount = Token::GetMinimumPrototokenCount()) = 0;

    Token& operator=(const Token& rhs);

    Token(const api::Core& core);
    Token(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);
    Token(const api::Core& core, const Purse& thePurse);

private:
    typedef Instrument ot_super;

    Token() = delete;
};
}  // namespace opentxs
#endif  // OT_CASH
#endif
