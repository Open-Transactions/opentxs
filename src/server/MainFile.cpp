// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "MainFile.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"

#include "Server.hpp"
#include "Transactor.hpp"

#include <irrxml/irrXML.hpp>

#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Mainfile::"

namespace opentxs::server
{

MainFile::MainFile(Server& server)
    : server_(server)
    , version_()
{
}

bool MainFile::SaveMainFileToString(String& strMainFile)
{
    Tag tag("notaryServer");

    // We're on version 2.0 since adding the master key.
    auto& cachedKey = server_.API().Crypto().DefaultKey();
    tag.add_attribute("version", cachedKey.IsGenerated() ? "2.0" : version_);
    tag.add_attribute("notaryID", server_.GetServerID().str());
    tag.add_attribute("serverNymID", server_.GetServerNym().ID().str());
    tag.add_attribute(
        "transactionNum",
        formatLong(server_.GetTransactor().transactionNumber()));

    if (cachedKey.IsGenerated())  // If it exists, then serialize it.
    {
        auto ascMasterContents = Armored::Factory();

        if (cachedKey.SerializeTo(ascMasterContents)) {
            tag.add_tag("cachedKey", ascMasterContents->Get());
        } else
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to write master key to notary file.")
                .Flush();
    }

    // Save the basket account information

    for (auto& it : server_.GetTransactor().idToBasketMap_) {
        auto strBasketID = String::Factory(it.first.c_str());
        auto strBasketAcctID = String::Factory(it.second.c_str());

        const auto BASKET_ACCOUNT_ID = Identifier::Factory(strBasketAcctID);
        auto BASKET_CONTRACT_ID = Identifier::Factory();

        bool bContractID =
            server_.GetTransactor().lookupBasketContractIDByAccountID(
                BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID);

        if (!bContractID) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Missing Contract ID for basket ID ")(
                strBasketID->Get())(".")
                .Flush();
            break;
        }

        auto strBasketContractID = String::Factory((BASKET_CONTRACT_ID));

        TagPtr pTag(new Tag("basketInfo"));

        pTag->add_attribute("basketID", strBasketID->Get());
        pTag->add_attribute("basketAcctID", strBasketAcctID->Get());
        pTag->add_attribute("basketContractID", strBasketContractID->Get());

        tag.add_tag(pTag);
    }

    server_.GetTransactor().voucherAccounts_.Serialize(tag);

    std::string str_result;
    tag.output(str_result);

    strMainFile.Concatenate("%s", str_result.c_str());

    return true;
}

// Setup the default location for the Sever Main File...
// maybe this should be set differently...
// should be set in the servers configuration.
//
bool MainFile::SaveMainFile()
{
    // Get the loaded (or new) version of the Server's Main File.
    //
    auto strMainFile = String::Factory();

    if (!SaveMainFileToString(strMainFile)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error saving to string. (Giving up on save attempt).")
            .Flush();
        return false;
    }
    // Try to save the notary server's main datafile to local storage...
    //
    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(strMainFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, "NOTARY"))  // todo
                                                          // hardcoding.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error saving notary (Failed writing armored string).")
            .Flush();
        return false;
    }
    // Save the Main File to the Harddrive... (or DB, if other storage module is
    // being used).
    //
    const bool bSaved = OTDB::StorePlainString(
        strFinal->Get(),
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        "");

    if (!bSaved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving main file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
    }
    return bSaved;
}

bool MainFile::CreateMainFile(
    const std::string& strContract,
    const std::string& strNotaryID,
    const std::string& strCert,
    const std::string& strNymID,
    const std::string& strCachedKey)
{
    if (!OTDB::StorePlainString(
            strContract,
            server_.API().DataFolder(),
            OTFolders::Contract().Get(),
            strNotaryID,
            "",
            "")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to store the server contract.")
            .Flush();
        return false;
    }

    if (!strCert.empty() && !OTDB::StorePlainString(
                                strCert,
                                server_.API().DataFolder(),
                                OTFolders::Cert().Get(),
                                strNymID,
                                "",
                                "")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to store the server Nym's public/private cert.")
            .Flush();
        return false;
    }

    const char* szBlankFile =  // todo hardcoding.
        "<notaryServer version=\"2.0\"\n"
        " notaryID=\"%s\"\n"
        " serverNymID=\"%s\"\n"
        " transactionNum=\"%ld\" >\n"
        "\n"
        "<cachedKey>\n"
        "%s</cachedKey>\n"
        "\n"
        "<accountList type=\"voucher\" count=\"0\" >\n"
        "\n"
        "</accountList>\n"
        "\n"
        "</notaryServer>\n\n";

    std::int64_t lTransNum = 5;  // a starting point, for the new server.

    auto strNotaryFile = String::Factory();
    strNotaryFile->Format(
        szBlankFile,
        strNotaryID.c_str(),
        strNymID.c_str(),
        lTransNum,
        strCachedKey.c_str());

    std::string str_Notary(strNotaryFile->Get());

    if (!OTDB::StorePlainString(
            str_Notary,
            server_.API().DataFolder(),
            ".",
            "notaryServer.xml",
            "",
            ""))  // todo hardcoding.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to store the new notaryServer.xml file.")
            .Flush();
        return false;
    }
    auto ascCachedKey = Armored::Factory();
    ascCachedKey->Set(strCachedKey.c_str());
    auto& cachedKey = server_.API().Crypto().LoadDefaultKey(ascCachedKey);

    if (!cachedKey.HasHashCheck()) {
        OTPassword tempPassword;
        tempPassword.zeroMemory();
        cachedKey.GetMasterPassword(
            cachedKey,
            tempPassword,
            "We do not have a check hash yet for this password, "
            "please enter your password",
            true);
        if (!SaveMainFile()) { OT_FAIL; }
    }
    // At this point, the contract is saved, the cert is saved, and the
    // notaryServer.xml file
    // is saved. All we have left is the Nymfile, which we'll create.

    auto loaded = server_.LoadServerNym(Identifier::Factory(strNymID));
    if (false == loaded) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Error loading server nym.")
            .Flush();
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": OKAY, we have apparently created the new "
            "server. "
            "Let's try to load up your new server contract...")
            .Flush();

        return true;
    }
    return false;
}

bool MainFile::LoadMainFile(bool bReadOnly)
{
    if (!OTDB::Exists(
            server_.API().DataFolder(),
            ".",
            server_.WalletFilename().Get(),
            "",
            "")) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error finding file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
        return false;
    }
    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (!strFileContents->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to read main file: ")(
            server_.WalletFilename().Get())(".")
            .Flush();
        return false;
    }

    bool bNeedToSaveAgain = false;

    bool bFailure = false;

    {
        auto xmlFileContents = StringXML::Factory(strFileContents);

        if (false == xmlFileContents->DecodeIfArmored()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Notary server file apparently was encoded and "
                "then failed decoding. Filename: ")(
                server_.WalletFilename().Get())(". Contents: ")(
                strFileContents->Get())(".")
                .Flush();
            return false;
        }

        irr::io::IrrXMLReader* xml =
            irr::io::createIrrXMLReader(xmlFileContents.get());
        std::unique_ptr<irr::io::IrrXMLReader> theXMLGuardian(xml);

        while (xml && xml->read()) {
            // strings for storing the data that we want to read out of the file
            auto AssetName = String::Factory();
            auto InstrumentDefinitionID = String::Factory();
            const auto strNodeName = String::Factory(xml->getNodeName());

            switch (xml->getNodeType()) {
                case irr::io::EXN_TEXT:
                    break;
                case irr::io::EXN_ELEMENT: {
                    if (strNodeName->Compare("notaryServer")) {
                        version_ = xml->getAttributeValue("version");
                        server_.SetNotaryID(Identifier::Factory(String::Factory(
                            xml->getAttributeValue("notaryID"))));
                        server_.SetServerNymID(
                            xml->getAttributeValue("serverNymID"));

                        auto strTransactionNumber =
                            String::Factory();  // The server issues
                                                // transaction numbers and
                                                // stores the counter here
                                                // for the latest one.
                        strTransactionNumber = String::Factory(
                            xml->getAttributeValue("transactionNum"));
                        server_.GetTransactor().transactionNumber(
                            strTransactionNumber->ToLong());
                        {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": Loading Open Transactions server. "
                                "File version: ")(version_)(
                                "Last Issued Transaction Number: ")(
                                server_.GetTransactor().transactionNumber())(
                                "Notary ID: ")(server_.GetServerID().str())(
                                "Server Nym ID: ")(server_.ServerNymID())(".")
                                .Flush();
                        }

                    } else if (strNodeName->Compare("cachedKey")) {
                        auto ascCachedKey = Armored::Factory();

                        if (Contract::LoadEncodedTextField(xml, ascCachedKey)) {
                            auto& cachedKey =
                                server_.API().Crypto().LoadDefaultKey(
                                    ascCachedKey);

                            if (!cachedKey.HasHashCheck()) {
                                OTPassword tempPassword;
                                tempPassword.zeroMemory();
                                bNeedToSaveAgain = cachedKey.GetMasterPassword(
                                    cachedKey,
                                    tempPassword,
                                    "We do not have a check hash yet for this "
                                    "password, please enter your password",
                                    true);
                            }
                        }
                        {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": Loading cachedKey: ")(ascCachedKey->Get())(
                                ".")
                                .Flush();
                        }

                    }
                    // the voucher reserve account IDs.
                    else if (strNodeName->Compare("accountList")) {
                        const auto strAcctType =
                            String::Factory(xml->getAttributeValue("type"));
                        const auto strAcctCount =
                            String::Factory(xml->getAttributeValue("count"));

                        if ((-1) == server_.GetTransactor()
                                        .voucherAccounts_.ReadFromXMLNode(
                                            xml, strAcctType, strAcctCount))
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": Error loading voucher accountList.")
                                .Flush();
                    } else if (strNodeName->Compare("basketInfo")) {
                        auto strBasketID =
                            String::Factory(xml->getAttributeValue("basketID"));
                        auto strBasketAcctID = String::Factory(
                            xml->getAttributeValue("basketAcctID"));
                        auto strBasketContractID = String::Factory(
                            xml->getAttributeValue("basketContractID"));
                        const auto BASKET_ID = Identifier::Factory(strBasketID),
                                   BASKET_ACCT_ID =
                                       Identifier::Factory(strBasketAcctID),
                                   BASKET_CONTRACT_ID =
                                       Identifier::Factory(strBasketContractID);

                        if (server_.GetTransactor().addBasketAccountID(
                                BASKET_ID,
                                BASKET_ACCT_ID,
                                BASKET_CONTRACT_ID)) {
                            LogNormal(OT_METHOD)(__FUNCTION__)(
                                ": Loading basket currency info... "
                                "Basket ID: ")(strBasketID)(
                                " Basket Acct ID: ")(strBasketAcctID)(
                                " Basket Contract ID: ")(strBasketContractID)(
                                ".")
                                .Flush();
                        } else {
                            LogOutput(OT_METHOD)(__FUNCTION__)(
                                ": Error adding basket currency info."
                                " Basket ID: ")(strBasketID->Get())(
                                ". Basket Acct ID: ")(strBasketAcctID->Get())(
                                ".")
                                .Flush();
                        }
                    } else {
                        // unknown element type
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Unknown element type: ")(xml->getNodeName())(".")
                            .Flush();
                    }
                } break;
                default: {
                }
            }
        }
    }

    if (server_.ServerNymID().empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to determine server nym id.")
            .Flush();
        bFailure = true;
    }

    if (false == bFailure) {
        const auto loaded =
            server_.LoadServerNym(Identifier::Factory(server_.ServerNymID()));

        if (false == loaded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load server nym.")
                .Flush();
            bFailure = true;
        }
    }

    if (false == bFailure) {
        const auto loaded = LoadServerUserAndContract();

        if (false == loaded) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load nym.").Flush();
            bFailure = true;
        }
    }

    if (false == bReadOnly) {
        if (bNeedToSaveAgain) { SaveMainFile(); }
    }

    return !bFailure;
}

bool MainFile::LoadServerUserAndContract()
{
    bool bSuccess = false;
    auto& serverNym = server_.m_nymServer;

    OT_ASSERT(!version_.empty());
    OT_ASSERT(!server_.GetServerID().str().empty());
    OT_ASSERT(!server_.ServerNymID().empty());

    serverNym =
        server_.API().Wallet().Nym(Identifier::Factory(server_.ServerNymID()));

    if (serverNym->HasCapability(NymCapability::SIGN_MESSAGE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Server nym is viable.").Flush();
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Server nym lacks private keys.")
            .Flush();

        return false;
    }

    // Load Cron (now that we have the server Nym.
    // (I WAS loading this erroneously in Server.Init(), before
    // the Nym had actually been loaded from disk. That didn't work.)
    //
    const auto NOTARY_ID = Identifier::Factory(server_.GetServerID());

    // Make sure the Cron object has a pointer to the server's Nym.
    // (For signing stuff...)
    //
    server_.Cron().SetNotaryID(NOTARY_ID);
    server_.Cron().SetServerNym(serverNym);

    if (!server_.Cron().LoadCron()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading Cron file. (Did you just create this "
            "server?).")
            .Flush();
    }
    LogNormal(OT_METHOD)(__FUNCTION__)(": Loading the server contract...")
        .Flush();
    auto pContract = server_.API().Wallet().Server(NOTARY_ID);

    if (pContract) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": ** Main Server Contract Verified **")
            .Flush();
        bSuccess = true;
    } else {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed reading Main Server Contract: ")
            .Flush();
    }

    return bSuccess;
}
}  // namespace opentxs::server
