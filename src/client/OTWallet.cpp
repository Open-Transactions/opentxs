// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/OTWallet.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Wallet.hpp"
#if OT_CASH
#include "opentxs/cash/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::OTWallet::"

namespace opentxs
{

OTWallet::OTWallet(const api::Core& api)
    : Lockable()
    , api_(api)
#if OT_CASH
    , m_pWithdrawalPurse(nullptr)
#endif
    , m_strName(String::Factory())
    , m_strVersion(String::Factory())
    , m_strFilename(String::Factory())
    , m_strDataFolder(String::Factory(api_.DataFolder()))
{
    // WARNING: do not access api_.Wallet() during construction
}

void OTWallet::release(const Lock&) {}

#if OT_CASH
// While waiting on server response to a withdrawal, we keep the private coin
// data here so we can unblind the response.
// This information is so important (as important as the digital cash token
// itself, until the unblinding is done) that we need to save the file right
// away.
void OTWallet::AddPendingWithdrawal(const Purse& thePurse)
{
    Lock lock(lock_);
    // TODO maintain a list here (I don't know why, the server response is
    // nearly
    // instant and then it's done.)

    // TODO notice I don't check the pointer here to see if it's already set, I
    // just start using it.. Fix that.
    m_pWithdrawalPurse = const_cast<Purse*>(&thePurse);
}  // TODO WARNING: If this data is lost before the transaction is completed,
   // the user will be unable to unblind his tokens and make them spendable.
   // So this data MUST be SAVED until the successful withdrawal is verified!

void OTWallet::RemovePendingWithdrawal()
{
    Lock lock(lock_);

    if (m_pWithdrawalPurse) delete m_pWithdrawalPurse;

    m_pWithdrawalPurse = nullptr;
}
#endif  // OT_CASH

std::string OTWallet::GetPhrase()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = api_.Storage().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        Lock lock(lock_);
        save_wallet(lock);
    }

    return api_.Seeds().Passphrase(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::GetSeed()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = api_.Storage().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        Lock lock(lock_);
        save_wallet(lock);
    }

    return api_.Seeds().Bip32Root(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::GetWords()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const std::string defaultFingerprint = api_.Storage().DefaultSeed();
    const bool firstTime = defaultFingerprint.empty();

    if (firstTime) {
        Lock lock(lock_);
        save_wallet(lock);
    }

    return api_.Seeds().Words(defaultFingerprint);
#else
    return "";
#endif
}

std::string OTWallet::ImportSeed(
    const OTPassword& words,
    const OTPassword& passphrase) const
{
#if OT_CRYPTO_WITH_BIP39
    return api_.Seeds().ImportSeed(words, passphrase);
#else
    return "";
#endif
}

#if OT_CASH
Purse* OTWallet::GetPendingWithdrawal()
{
    Lock lock(lock_);

    return m_pWithdrawalPurse;
}
#endif

void OTWallet::DisplayStatistics(String& strOutput) const
{
    Lock lock(lock_);
    strOutput.Concatenate(
        "\n-------------------------------------------------\n");
    strOutput.Concatenate("WALLET STATISTICS:\n");

    strOutput.Concatenate("\nNYM(s):\n\n");

    for (auto& it : api_.Storage().LocalNyms()) {
        const auto& nymId = Identifier::Factory(it);
        const auto& pNym = api_.Wallet().Nym(nymId);

        OT_ASSERT(pNym);

        pNym->DisplayStatistics(strOutput);
    }

    strOutput.Concatenate(
        "-------------------------------------------------\n");
    strOutput.Concatenate("ACCOUNTS:\n\n");

    for (const auto& it : api_.Storage().AccountList()) {
        const auto& accountID = it.first;
        const auto account =
            api_.Wallet().Account(Identifier::Factory(accountID));
        account.get().DisplayStatistics(strOutput);
        strOutput.Concatenate(
            "-------------------------------------------------\n\n");
    }
}

bool OTWallet::save_contract(const Lock& lock, String& strContract)
{
    OT_ASSERT(verify_lock(lock))

    Tag tag("wallet");

    // Name is in the clear in memory,
    // and base64 in storage.
    auto ascName = Armored::Factory();

    if (m_strName->Exists()) {
        ascName->SetString(m_strName, false);  // linebreaks == false
    }

    auto& cachedKey = api_.Crypto().DefaultKey();
    tag.add_attribute("name", m_strName->Exists() ? ascName->Get() : "");
    tag.add_attribute(
        "version", cachedKey.IsGenerated() ? "2.0" : m_strVersion->Get());

    if (cachedKey.IsGenerated())  // If it exists, then serialize it.
    {
        auto ascMasterContents = Armored::Factory();

        if (cachedKey.SerializeTo(ascMasterContents)) {
            tag.add_tag("cachedKey", ascMasterContents->Get());
        } else
            otErr << "OTWallet::SaveContract: Failed trying to write master "
                     "key to wallet.\n";
    }

    std::string str_result;
    tag.output(str_result);
    strContract.Concatenate("%s", str_result.c_str());

    return true;
}

bool OTWallet::Encrypt_ByKeyID(
    const std::string&,
    const String&,
    String&,
    const String&,
    bool)
{
    return false;
}

bool OTWallet::Decrypt_ByKeyID(
    const std::string&,
    const String&,
    String&,
    const String&)
{
    return false;
}

// Pass in the name only, NOT the full path. If you pass nullptr, it remembers
// full path from last time. (Better to do that.)
bool OTWallet::save_wallet(const Lock& lock, const char* szFilename)
{
    OT_ASSERT(verify_lock(lock))

    if (nullptr != szFilename) m_strFilename->Set(szFilename);

    if (!m_strFilename->Exists()) {
        otErr << __FUNCTION__ << ": Filename Dosn't Exist!\n";
        OT_FAIL;
    }

    bool bSuccess = false;
    auto strContract = String::Factory();

    if (save_contract(lock, strContract)) {

        // Try to save the wallet to local storage.
        //
        auto strFinal = String::Factory();
        auto ascTemp = Armored::Factory(strContract);

        if (false == ascTemp->WriteArmoredString(
                         strFinal, "WALLET"))  // todo hardcoding.
        {
            otErr << "OTWallet::SaveWallet: Error saving wallet (failed "
                     "writing armored string):\n"
                  << m_strDataFolder << Log::PathSeparator() << m_strFilename
                  << "\n";
            return false;
        }

        // Wallet file is the only one in data_folder (".") and not a subfolder
        // of that.
        bSuccess = OTDB::StorePlainString(
            strFinal->Get(),
            api_.DataFolder(),
            ".",
            m_strFilename->Get(),
            "",
            "");  // <==== Store
                  // Plain String
    }

    return bSuccess;
}

// Pass in the name only, NOT the full path. If you pass nullptr, it remembers
// full path from last time. (Better to do that.)
bool OTWallet::SaveWallet(const char* szFilename)
{
    Lock lock(lock_);

    return save_wallet(lock, szFilename);
}
/*

<wallet name="" version="2.0">

<cachedKey>
CkwAAQCAAAD//wAAAAhVRpwTzc+1NAAAABCKe14aROG8v/ite3un3bBCAAAAINyw
HXTM/x449Al2z8zBHBTRF77jhHkYLj8MIgqrJ2Ep
</cachedKey>

</wallet>

 */
bool OTWallet::LoadWallet(const char* szFilename)
{
    OT_ASSERT_MSG(
        m_strFilename->Exists() || (nullptr != szFilename),
        "OTWallet::LoadWallet: nullptr filename.\n");

    Lock lock(lock_);
    release(lock);

    // The directory is "." because unlike every other OT file, the wallet file
    // doesn't go into a subdirectory, but it goes into the main data_folder
    // itself.
    // Every other file, however, needs to specify its folder AND filename (and
    // both
    // of those will be appended to the local path to form the complete file
    // path.)
    //
    if (!m_strFilename->Exists())  // If it's not already set, then set it.
        m_strFilename->Set(szFilename);  // (We know nullptr wasn't passed in,
                                         // in this case.)

    if (nullptr == szFilename)  // If nullptr was passed in, then set the
                                // pointer to existing string.
        szFilename = m_strFilename->Get();  // (We know existing string is
                                            // there, in this case.)

    if (!OTDB::Exists(api_.DataFolder(), ".", szFilename, "", "")) {
        otErr << __FUNCTION__ << ": Wallet file does not exist: " << szFilename
              << ". Creating...\n";

        const char* szContents = "<wallet name=\"\" version=\"1.0\">\n"
                                 "\n"
                                 "</wallet>\n";

        if (!OTDB::StorePlainString(
                szContents, api_.DataFolder(), ".", szFilename, "", "")) {
            otErr << __FUNCTION__
                  << ": Error: Unable to create blank wallet file.\n";
            OT_FAIL;
        }
    }

    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        api_.DataFolder(), ".", szFilename, "", ""));  // <===
                                                       // LOADING
                                                       // FROM
                                                       // DATA
                                                       // STORE.

    if (!strFileContents->Exists()) {
        otErr << __FUNCTION__ << ": Error reading wallet file: " << szFilename
              << "\n";
        return false;
    }

    bool bNeedToSaveAgain = false;

    {
        auto xmlFileContents = StringXML::Factory(strFileContents);

        if (!xmlFileContents->DecodeIfArmored()) {
            otErr << __FUNCTION__
                  << ": Input string apparently was encoded and then failed "
                     "decoding. Filename: "
                  << szFilename
                  << " \n"
                     "Contents: \n"
                  << strFileContents << "\n";
            return false;
        }

        irr::io::IrrXMLReader* xml =
            irr::io::createIrrXMLReader(xmlFileContents.get());

        // parse the file until end reached
        while (xml && xml->read()) {
            // strings for storing the data that we want to read out of the file
            auto NymName = String::Factory();
            auto NymID = String::Factory();
            auto AssetName = String::Factory();
            auto InstrumentDefinitionID = String::Factory();
            auto ServerName = String::Factory();
            auto NotaryID = String::Factory();
            auto AcctName = String::Factory();
            auto AcctID = String::Factory();
            const auto strNodeName = String::Factory(xml->getNodeName());

            switch (xml->getNodeType()) {
                case irr::io::EXN_NONE:
                case irr::io::EXN_TEXT:
                case irr::io::EXN_COMMENT:
                case irr::io::EXN_ELEMENT_END:
                case irr::io::EXN_CDATA:
                    // in this xml file, the only text which occurs is the
                    // messageText
                    // messageText = xml->getNodeData();
                    break;
                case irr::io::EXN_ELEMENT: {
                    if (strNodeName->Compare("wallet")) {
                        auto ascWalletName =
                            Armored::Factory(xml->getAttributeValue("name"));

                        if (ascWalletName->Exists())
                            ascWalletName->GetString(
                                m_strName,
                                false);  // linebreaks == false

                        //                      m_strName            =
                        // xml->getAttributeValue("name");
                        //                      OTLog::OTPath        =
                        // xml->getAttributeValue("path");
                        m_strVersion =
                            String::Factory(xml->getAttributeValue("version"));

                        LogDetail(OT_METHOD)(__FUNCTION__)(
                            ": Loading wallet: ")(m_strName)(", version: ")(
                            m_strVersion)
                            .Flush();
                    } else if (strNodeName->Compare("cachedKey")) {
                        auto ascCachedKey = Armored::Factory();

                        if (Contract::LoadEncodedTextField(xml, ascCachedKey)) {
                            // We successfully loaded the cachedKey from file,
                            // so let's SET it as the cached key globally...
                            auto& cachedKey =
                                api_.Crypto().LoadDefaultKey(ascCachedKey);

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

                        LogDetail(OT_METHOD)(__FUNCTION__)(
                            "Loading cachedKey: ")(ascCachedKey)
                            .Flush();
                    } else if (strNodeName->Compare("account")) {
                        auto ascAcctName =
                            Armored::Factory(xml->getAttributeValue("name"));

                        if (ascAcctName->Exists())
                            ascAcctName->GetString(
                                AcctName,
                                false);  // linebreaks == false

                        AcctID = String::Factory(
                            xml->getAttributeValue("accountID"));
                        NotaryID =
                            String::Factory(xml->getAttributeValue("notaryID"));
                        LogVerbose(OT_METHOD)(__FUNCTION__)(
                            " ------------------------------------------")(
                            "----")("---------------------------- ")
                            .Flush();
                        LogVerbose(OT_METHOD)(__FUNCTION__)(
                            "****Account**** (wallet listing) ")
                            .Flush();
                        LogVerbose(OT_METHOD)(__FUNCTION__)(" Account Name: ")(
                            AcctName)
                            .Flush();
                        LogVerbose(OT_METHOD)(__FUNCTION__)("   Account ID: ")(
                            AcctID)
                            .Flush();
                        LogVerbose(OT_METHOD)(__FUNCTION__)("     Notary ID: ")(
                            NotaryID)
                            .Flush();
                        const auto ACCOUNT_ID = Identifier::Factory(AcctID),
                                   NOTARY_ID = Identifier::Factory(NotaryID);
                        std::unique_ptr<Account> pAccount(
                            Account::LoadExistingAccount(
                                api_, ACCOUNT_ID, NOTARY_ID));

                        if (pAccount) {
                            pAccount->SetName(AcctName);
                            api_.Wallet().ImportAccount(pAccount);
                        } else {
                            otErr
                                << __FUNCTION__
                                << ": Error loading existing Asset Account.\n";
                        }
                    }
                    // This tag is no longer saved in the wallet, but it is
                    // still parsed to allow conversion of existing wallets.
                    // From now on, the BIP39 class tracks the last used index
                    // individually for each seed rather than globally in the
                    // wallet (which assumed only one seed existed).
                    else if (strNodeName->Compare("hd")) {
#if OT_CRYPTO_SUPPORTED_KEY_HD
                        std::uint32_t index = String::StringToUint(
                            xml->getAttributeValue("index"));
                        // An empty string will load the default seed
                        std::string seed = "";
                        api_.Seeds().UpdateIndex(seed, index);
#endif
                    } else {
                        // unknown element type
                        otErr << __FUNCTION__ << ": unknown element type: "
                              << xml->getNodeName() << "\n";
                    }
                } break;
                default:
                    LogInsane(OT_METHOD)(__FUNCTION__)(": Unknown XML type: ")(
                        xml->getNodeName())
                        .Flush();
                    break;
            }
        }  // while xml->read()

        //
        // delete the xml parser after usage
        if (xml) delete xml;
    }

    // In case we converted any of the Nyms to the new "master key" encryption.
    if (bNeedToSaveAgain) save_wallet(lock, szFilename);

    return true;
}

OTWallet::~OTWallet()
{
    Lock lock(lock_);
    release(lock);
}
}  // namespace opentxs
