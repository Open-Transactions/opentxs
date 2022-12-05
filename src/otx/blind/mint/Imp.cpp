// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/blind/mint/Imp.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <chrono>
#include <compare>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <utility>

#include "internal/api/Legacy.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "otx/common/OTStorage.hpp"

namespace fs = std::filesystem;

namespace opentxs::otx::blind::mint
{
Mint::Mint(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::Nym& serverNym,
    const identifier::UnitDefinition& unit)
    : Imp(api)
    , private_()
    , public_()
    , notary_id_(notary)
    , server_nym_id_(serverNym)
    , instrument_definition_id_(unit)
    , denomination_count_(0)
    , save_private_keys_(false)
    , series_(0)
    , valid_from_(Time::min())
    , valid_to_(Time::min())
    , expiration_(Time::min())
    , cash_account_id_()
{
    foldername_->Set(api.Internal().Legacy().Mint());
    filename_->Set(api_.Internal()
                       .Legacy()
                       .MintFileName(notary_id_, instrument_definition_id_)
                       .string()
                       .c_str());
    InitMint();
}

Mint::Mint(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit)
    : Imp(api, unit)
    , private_()
    , public_()
    , notary_id_(notary)
    , server_nym_id_()
    , instrument_definition_id_(unit)
    , denomination_count_(0)
    , save_private_keys_(false)
    , series_(0)
    , valid_from_(Time::min())
    , valid_to_(Time::min())
    , expiration_(Time::min())
    , cash_account_id_()
{
    foldername_->Set(api.Internal().Legacy().Mint());
    filename_->Set(api_.Internal()
                       .Legacy()
                       .MintFileName(notary_id_, instrument_definition_id_)
                       .string()
                       .c_str());
    InitMint();
}

Mint::Mint(const api::Session& api)
    : Imp(api)
    , private_()
    , public_()
    , notary_id_()
    , server_nym_id_()
    , instrument_definition_id_()
    , denomination_count_(0)
    , save_private_keys_(false)
    , series_(0)
    , valid_from_(Time::min())
    , valid_to_(Time::min())
    , expiration_(Time::min())
    , cash_account_id_()
{
    InitMint();
}

// Verify the current date against the VALID FROM / EXPIRATION dates.
// (As opposed to tokens, which are verified against the valid from/to dates.)
auto Mint::Expired() const -> bool
{
    const auto CURRENT_TIME = Clock::now();

    if ((CURRENT_TIME >= valid_from_) && (CURRENT_TIME <= expiration_)) {
        return false;
    } else {
        return true;
    }
}

void Mint::ReleaseDenominations()
{
    public_.clear();
    private_.clear();
}

// If you want to load a certain Mint from string, then
// you will call LoadContractFromString() (say). Well this Releases the
// contract,
// before loading it, which calls InitMint() to zero out all the important
// pieces of
// data.
//
void Mint::Release_Mint()
{
    ReleaseDenominations();
    cash_account_id_.clear();
    Imp::Release_Mint();
}

void Mint::Release()
{
    Release_Mint();
    // I overrode the parent, so now I give him a chance to clean up.
    Contract::Release();
    InitMint();
}

void Mint::InitMint()
{
    contract_type_->Set("MINT");

    denomination_count_ = 0;

    save_private_keys_ =
        false;  // Determines whether it serializes private keys (no if false)

    // Mints expire and new ones are rotated in.
    // All tokens have the same series, and validity dates,
    // of the mint that created them.
    series_ = 0;
    valid_from_ = Time::min();
    valid_to_ = Time::min();
    expiration_ = Time::min();
}

auto Mint::LoadContract() -> bool { return LoadMint({}); }

auto Mint::LoadMint(std::string_view extension) -> bool
{
    if (!foldername_->Exists()) {
        foldername_->Set(api_.Internal().Legacy().Mint());
    }

    const auto strNotaryID = String::Factory(notary_id_);

    if (!filename_->Exists()) {
        filename_->Set(
            api_.Internal()
                .Legacy()
                .MintFileName(notary_id_, instrument_definition_id_, extension)
                .string()
                .c_str());
    }

    const auto strFilename =
        fs::path{instrument_definition_id_.asBase58(api_.Crypto())} +=
        extension;
    const char* szFolder1name = api_.Internal().Legacy().Mint();
    const char* szFolder2name = strNotaryID->Get();
    const auto pathString = strFilename.string();
    const char* szFilename = pathString.c_str();

    if (!OTDB::Exists(
            api_,
            api_.DataFolder().string(),
            szFolder1name,
            szFolder2name,
            szFilename,
            "")) {
        LogDetail()(OT_PRETTY_CLASS())("File does not exist: ")(
            szFolder1name)('/')(szFolder2name)('/')(szFilename)
            .Flush();
        return false;
    }

    UnallocatedCString strFileContents(OTDB::QueryPlainString(
        api_,
        api_.DataFolder().string(),
        szFolder1name,
        szFolder2name,
        szFilename,
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (strFileContents.length() < 2) {
        LogError()(OT_PRETTY_CLASS())("Error reading file: ")(
            szFolder1name)('/')(szFolder2name)('/')(szFilename)(".")
            .Flush();
        return false;
    }

    // NOTE: No need to worry about the OT ARMORED file format, since
    // LoadContractFromString already handles that internally.

    auto strRawFile = String::Factory(strFileContents.c_str());

    bool bSuccess = LoadContractFromString(
        strRawFile);  // Note: This handles OT ARMORED file format.

    return bSuccess;
}

auto Mint::SaveMint(std::string_view extension) -> bool
{
    if (!foldername_->Exists()) {
        foldername_->Set(api_.Internal().Legacy().Mint());
    }

    const auto strNotaryID = String::Factory(notary_id_);

    if (!filename_->Exists()) {
        filename_->Set(
            api_.Internal()
                .Legacy()
                .MintFileName(notary_id_, instrument_definition_id_, extension)
                .string()
                .c_str());
    }

    const auto strFilename =
        fs::path{instrument_definition_id_.asBase58(api_.Crypto())} +=
        extension;
    const char* szFolder1name = api_.Internal().Legacy().Mint();
    const char* szFolder2name = strNotaryID->Get();
    const auto pathString = strFilename.string();
    const char* szFilename = pathString.c_str();
    auto strRawFile = String::Factory();

    if (!SaveContractRaw(strRawFile)) {
        LogError()(OT_PRETTY_CLASS())(" Error saving Mintfile (to string): ")(
            szFolder1name)('/')(szFolder2name)('/')(szFilename)(".")
            .Flush();
        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(strRawFile);

    if (false == ascTemp->WriteArmoredString(strFinal, contract_type_->Get())) {
        LogError()(OT_PRETTY_CLASS())(
            " Error saving mint (Failed writing armored string): ")(
            szFolder1name)('/')(szFolder2name)('/')(szFilename)(".")
            .Flush();
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        api_,
        strFinal->Get(),
        api_.DataFolder().string(),
        szFolder1name,
        szFolder2name,
        szFilename,
        "");  // <=== SAVING TO LOCAL DATA STORE.
    if (!bSaved) {
        LogError()(OT_PRETTY_CLASS())("Error writing to file: ")(
            szFolder1name)('/')(szFolder2name)('/')(strFilename)
            .Flush();

        return false;
    }

    return true;
}

// Make sure this contract checks out. Very high level.
// Verifies ID and signature.
auto Mint::VerifyMint(const identity::Nym& theOperator) -> bool
{
    // Make sure that the supposed Contract ID that was set is actually
    // a hash of the contract file, signatures and all.
    if (!VerifyContractID()) {
        LogError()(OT_PRETTY_CLASS())(
            " Error comparing Mint ID to Asset Contract ID.")
            .Flush();
        return false;
    } else if (!VerifySignature(theOperator)) {
        LogError()(OT_PRETTY_CLASS())("Error verifying signature on mint.")
            .Flush();
        return false;
    }

    LogDebug()(OT_PRETTY_CLASS())("We now know that...").Flush();
    LogDebug()(OT_PRETTY_CLASS())("1. The Asset Contract ID matches the "
                                  "Mint ID loaded from the Mint file.")
        .Flush();
    LogDebug()(OT_PRETTY_CLASS())("2. The SIGNATURE VERIFIED.").Flush();
    return true;
}

// Unlike other contracts, which calculate their ID from a hash of the file
// itself, a mint has
// the same ID as its Asset Contract.  When we open the Mint file, we read the
// Instrument Definition ID
// from it and then verify that it matches what we were expecting from the asset
// type.
auto Mint::VerifyContractID() const -> bool
{
    // I use the == operator here because there is no != operator at this time.
    // That's why you see the ! outside the parenthesis.
    if (!(id_ == instrument_definition_id_)) {
        auto str1 = String::Factory(id_),
             str2 = String::Factory(instrument_definition_id_);

        LogError()(OT_PRETTY_CLASS())(
            " Mint ID does NOT match Instrument Definition. ")(str1.get())(
            " | ")(str2.get())(".")
            .Flush();
        //                "\nRAW FILE:\n--->" << raw_file_ << "<---"
        return false;
    } else {
        auto str1 = String::Factory(id_);
        LogVerbose()(OT_PRETTY_CLASS())(
            " Mint ID *SUCCESSFUL* match to Asset Contract ID: ")(str1.get())
            .Flush();
        return true;
    }
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
auto Mint::GetPrivate(Armored& theArmor, const Amount& lDenomination) const
    -> bool
{
    try {
        theArmor.Set(private_.at(lDenomination));

        return true;
    } catch (...) {
        LogTrace()(OT_PRETTY_CLASS())("Denomination ")(
            lDenomination)(" not found")
            .Flush();

        return false;
    }
}

// The mint has a different key pair for each denomination.
// Pass in the actual denomination such as 5, 10, 20, 50, 100...
auto Mint::GetPublic(Armored& theArmor, const Amount& lDenomination) const
    -> bool
{
    try {
        theArmor.Set(public_.at(lDenomination));

        return true;
    } catch (...) {
        LogTrace()(OT_PRETTY_CLASS())("Denomination ")(
            lDenomination)(" not found")
            .Flush();

        return false;
    }
}

// If you need to withdraw a specific amount, pass it in here and the
// mint will return the largest denomination that is equal to or smaller
// than the amount.
// Then you can subtract the denomination from the amount and call this method
// again, and again, until it reaches 0, in order to create all the necessary
// tokens to reach the full withdrawal amount.
auto Mint::GetLargestDenomination(const Amount& lAmount) const -> Amount
{
    for (std::int32_t nIndex = GetDenominationCount() - 1; nIndex >= 0;
         nIndex--) {
        const Amount& lDenom = GetDenomination(nIndex);

        if (lDenom <= lAmount) { return lDenom; }
    }

    return 0;
}

// If you call GetDenominationCount, you can then use this method
// to look up a denomination by index.
// You could also iterate through them by index.
auto Mint::GetDenomination(std::int32_t nIndex) const -> Amount
{
    // index out of bounds.
    if (nIndex > (denomination_count_ - 1)) { return 0; }

    std::int32_t nIterateIndex = 0;

    for (auto it = public_.begin(); it != public_.end();
         ++it, nIterateIndex++) {

        if (nIndex == nIterateIndex) { return it->first; }
    }

    return 0;
}

// The default behavior of this function does NOT save the private keys. It only
// serializes the public keys, and it is safe to send the object to the client.
// If the server needs to save the private keys, then call SetSavePrivateKeys()
// first.
void Mint::UpdateContents(const PasswordPrompt& reason)
{
    auto NOTARY_ID = String::Factory(notary_id_),
         NOTARY_NYM_ID = String::Factory(server_nym_id_),
         INSTRUMENT_DEFINITION_ID = String::Factory(instrument_definition_id_),
         CASH_ACCOUNT_ID = String::Factory(cash_account_id_);

    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    Tag tag("mint");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute("serverNymID", NOTARY_NYM_ID->Get());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("cashAcctID", CASH_ACCOUNT_ID->Get());
    tag.add_attribute("series", std::to_string(series_));
    tag.add_attribute("expiration", formatTimestamp(expiration_));
    tag.add_attribute("validFrom", formatTimestamp(valid_from_));
    tag.add_attribute("validTo", formatTimestamp(valid_to_));

    if (denomination_count_) {
        if (save_private_keys_) {
            save_private_keys_ = false;  // reset this back to false again. Use
                                         // SetSavePrivateKeys() to set it true.

            for (auto& it : private_) {
                TagPtr tagPrivateInfo(
                    new Tag("mintPrivateInfo", it.second->Get()));
                tagPrivateInfo->add_attribute("denomination", [&] {
                    auto amount = UnallocatedCString{};
                    it.first.Serialize(writer(amount));
                    return amount;
                }());
                tag.add_tag(tagPrivateInfo);
            }
        }
        for (auto& it : public_) {
            TagPtr tagPublicInfo(new Tag("mintPublicInfo", it.second->Get()));
            tagPublicInfo->add_attribute("denomination", [&] {
                auto amount = UnallocatedCString{};
                it.first.Serialize(writer(amount));
                return amount;
            }());
            tag.add_tag(tagPublicInfo);
        }
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Mint::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    const auto strNodeName = String::Factory(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = ot_super::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (strNodeName->Compare("mint")) {
        auto strNotaryID = String::Factory(),
             strServerNymID = String::Factory(),
             strInstrumentDefinitionID = String::Factory(),
             strCashAcctID = String::Factory();

        version_ = String::Factory(xml->getAttributeValue("version"));
        strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        strServerNymID = String::Factory(xml->getAttributeValue("serverNymID"));
        strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        strCashAcctID = String::Factory(xml->getAttributeValue("cashAcctID"));

        series_ = atoi(xml->getAttributeValue("series"));
        expiration_ = parseTimestamp(xml->getAttributeValue("expiration"));

        valid_from_ = parseTimestamp(xml->getAttributeValue("validFrom"));
        valid_to_ = parseTimestamp(xml->getAttributeValue("validTo"));

        notary_id_ = api_.Factory().NotaryIDFromBase58(strNotaryID->Bytes());
        server_nym_id_ =
            api_.Factory().NymIDFromBase58(strServerNymID->Bytes());
        instrument_definition_id_ =
            api_.Factory().UnitIDFromBase58(strInstrumentDefinitionID->Bytes());
        cash_account_id_ =
            api_.Factory().IdentifierFromBase58(strCashAcctID->Bytes());

        LogDetail()(OT_PRETTY_CLASS())
            //    "\n===> Loading XML for mint into memory structures..."
            ("Mint version: ")(version_.get())(" Notary ID: ")(
                strNotaryID.get())(" Instrument Definition ID: ")(
                strInstrumentDefinitionID.get())(" Cash Acct ID: ")(
                strCashAcctID.get())(
                (cash_account_id_.empty()) ? "FAILURE" : "SUCCESS")(
                " loading Cash Account into memory for pointer: ")(
                "Mint::reserve_acct_ ")(" Series: ")(series_)(" Expiration: ")(
                expiration_)(" Valid From: ")(valid_from_)(" Valid To: ")(
                valid_to_)
                .Flush();

        nReturnVal = 1;
    } else if (strNodeName->Compare("mintPrivateInfo")) {
        auto lDenomination =
            factory::Amount(xml->getAttributeValue("denomination"));
        auto pArmor = Armored::Factory();

        if (!LoadEncodedTextField(xml, pArmor) || !pArmor->Exists()) {
            LogError()(OT_PRETTY_CLASS())("Error: mintPrivateInfo field "
                                          "without value.")
                .Flush();

            return (-1);  // error condition
        } else {
            LogTrace()(OT_PRETTY_CLASS())(
                " Loading private key for denomination ")(lDenomination)
                .Flush();
            private_.emplace(lDenomination, std::move(pArmor));
        }

        return 1;
    } else if (strNodeName->Compare("mintPublicInfo")) {
        auto lDenomination =
            factory::Amount(xml->getAttributeValue("denomination"));

        auto pArmor = Armored::Factory();

        if (!LoadEncodedTextField(xml, pArmor) || !pArmor->Exists()) {
            LogError()(OT_PRETTY_CLASS())("Error: mintPublicInfo field "
                                          "without value.")
                .Flush();

            return (-1);  // error condition
        } else {
            LogTrace()(OT_PRETTY_CLASS())(
                " Loading public key for denomination ")(lDenomination)
                .Flush();
            public_.emplace(lDenomination, std::move(pArmor));
            // Whether client or server, both sides have public. Each public
            // denomination should increment this count.
            denomination_count_++;
        }

        return 1;
    }

    return nReturnVal;
}

/*
 // Just make sure theMessage has these members populated:
 //
 // theMessage.nym_id_;
 // theMessage.instrument_definition_id_;
 // theMessage.notary_id_;

 // static method (call it without an instance, using notation:
 OTAccount::GenerateNewAccount)
 OTAccount * OTAccount::GenerateNewAccount(    const identifier::Generic&
 theNymID, const identifier::Notary& theNotaryID, const Nym & theServerNym,
 const OTMessage & theMessage,
                                            const OTAccount::AccountType
 eAcctType=OTAccount::user)


 // The above method uses this one internally...
 bool OTAccount::GenerateNewAccount(const Nym & theServer, const
 OTMessage & theMessage,
                                    const OTAccount::AccountType
 eAcctType=OTAccount::user)


 OTAccount * pAcct = nullptr;
 pAcct = OTAccount::LoadExistingAccount(ACCOUNT_ID, NOTARY_ID);
 */

// Lucre step 1: generate new mint
// Make sure the issuer here has a private key
// theMint.GenerateNewMint(nSeries, VALID_FROM, VALID_TO,
// INSTRUMENT_DEFINITION_ID, nym_server_,
// 1, 5, 10, 20, 50, 100, 500, 1000, 10000, 100000);
void Mint::GenerateNewMint(
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
    const PasswordPrompt& reason)
{
    Release();
    instrument_definition_id_ = theInstrumentDefinitionID;
    notary_id_ = theNotaryID;
    const auto& NOTARY_NYM_ID = theNotary.ID();
    server_nym_id_ = NOTARY_NYM_ID;
    series_ = nSeries;
    valid_from_ = VALID_FROM;
    valid_to_ = VALID_TO;
    expiration_ = MINT_EXPIRATION;
    auto account = wallet.Internal().CreateAccount(
        NOTARY_NYM_ID,
        theNotaryID,
        theInstrumentDefinitionID,
        theNotary,
        Account::mint,
        0,
        reason);

    if (account) {
        account.get().GetIdentifier(cash_account_id_);
        LogDetail()(OT_PRETTY_CLASS())(
            " Successfully created cash reserve account for new mint.")
            .Flush();
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            " Error creating cash reserve account for new mint.")
            .Flush();
    }

    account.Release();

    if (0 != nDenom1) { AddDenomination(theNotary, nDenom1, keySize, reason); }
    if (0 != nDenom2) { AddDenomination(theNotary, nDenom2, keySize, reason); }
    if (0 != nDenom3) { AddDenomination(theNotary, nDenom3, keySize, reason); }
    if (0 != nDenom4) { AddDenomination(theNotary, nDenom4, keySize, reason); }
    if (0 != nDenom5) { AddDenomination(theNotary, nDenom5, keySize, reason); }
    if (0 != nDenom6) { AddDenomination(theNotary, nDenom6, keySize, reason); }
    if (0 != nDenom7) { AddDenomination(theNotary, nDenom7, keySize, reason); }
    if (0 != nDenom8) { AddDenomination(theNotary, nDenom8, keySize, reason); }
    if (0 != nDenom9) { AddDenomination(theNotary, nDenom9, keySize, reason); }
    if (0 != nDenom10) {
        AddDenomination(theNotary, nDenom10, keySize, reason);
    }
}

Mint::~Mint() = default;
}  // namespace opentxs::otx::blind::mint
