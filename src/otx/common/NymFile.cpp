// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "otx/common/NymFile.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/crypto/OTSignedFile.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/common/OTStorage.hpp"

#define NYMFILE_VERSION "1.1"

namespace opentxs
{
auto Factory::NymFile(const api::Session& api, Nym_p targetNym, Nym_p signerNym)
    -> internal::NymFile*
{
    return new implementation::NymFile(api, targetNym, signerNym);
}
}  // namespace opentxs

namespace opentxs::implementation
{
NymFile::NymFile(const api::Session& api, Nym_p targetNym, Nym_p signerNym)
    : api_{api}
    , target_nym_{targetNym}
    , signer_nym_{signerNym}
    , m_lUsageCredits(0)
    , m_bMarkForDeletion(false)
    , m_strNymFile(String::Factory())
    , m_strVersion(String::Factory(NYMFILE_VERSION))
    , m_strDescription(String::Factory())
    , m_mapInboxHash()
    , m_mapOutboxHash()
    , m_dequeOutpayments()
    , m_setAccounts()
{
}

/// a payments message is a form of transaction, transported via Nymbox
void NymFile::AddOutpayments(std::shared_ptr<Message> theMessage)
{
    eLock lock(shared_lock_);

    m_dequeOutpayments.push_front(theMessage);
}

void NymFile::ClearAll()
{
    eLock lock(shared_lock_);

    m_mapInboxHash.clear();
    m_mapOutboxHash.clear();
    m_setAccounts.clear();
    m_dequeOutpayments.clear();
}

auto NymFile::CompareID(const identifier::Nym& rhs) const -> bool
{
    sLock lock(shared_lock_);

    return rhs == target_nym_->ID();
}

auto NymFile::DeserializeNymFile(
    const String& strNym,
    bool& converted,
    String::Map* pMapCredentials,
    const OTPassword* pImportPassword) -> bool
{
    sLock lock(shared_lock_);

    return deserialize_nymfile(
        lock, strNym, converted, pMapCredentials, pImportPassword);
}

template <typename T>
auto NymFile::deserialize_nymfile(
    const T& lock,
    const opentxs::String& strNym,
    bool& converted,
    opentxs::String::Map* pMapCredentials,
    const OTPassword* pImportPassword) -> bool
{
    OT_ASSERT(verify_lock(lock));

    bool bSuccess = false;
    bool convert = false;
    converted = false;
    //?    ClearAll();  // Since we are loading everything up... (credentials
    // are NOT cleared here. See note in Nym::ClearAll.)
    auto strNymXML = StringXML::Factory(strNym);  // todo optimize
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader(strNymXML.get());
    OT_ASSERT(nullptr != xml);
    std::unique_ptr<irr::io::IrrXMLReader> theCleanup(xml);

    // parse the file until end reached
    while (xml && xml->read()) {

        // strings for storing the data that we want to read out of the file
        //
        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
            case irr::io::EXN_TEXT:
            case irr::io::EXN_COMMENT:
            case irr::io::EXN_ELEMENT_END:
            case irr::io::EXN_CDATA:
                break;
            case irr::io::EXN_ELEMENT: {
                const auto strNodeName = String::Factory(xml->getNodeName());

                if (strNodeName->Compare("nymData")) {
                    m_strVersion =
                        String::Factory(xml->getAttributeValue("version"));
                    const auto UserNymID =
                        String::Factory(xml->getAttributeValue("nymID"));

                    // Server-side only...
                    auto strCredits =
                        String::Factory(xml->getAttributeValue("usageCredits"));

                    if (strCredits->GetLength() > 0)
                        m_lUsageCredits = strCredits->ToLong();
                    else
                        m_lUsageCredits = 0;  // This is the default anyway, but
                                              // just being safe...

                    if (UserNymID->GetLength()) {
                        LogDebug()(OT_PRETTY_CLASS())(
                            "Loading user, version: ")(
                            m_strVersion)(" NymID: ")(UserNymID)
                            .Flush();
                    }
                    bSuccess = true;
                    convert = (String::Factory("1.0")->Compare(m_strVersion));

                    if (convert) {
                        LogError()(OT_PRETTY_CLASS())(
                            "Converting nymfile with version ")(
                            m_strVersion)(".")
                            .Flush();
                    } else {
                        LogDetail()(OT_PRETTY_CLASS())(
                            "Not converting nymfile because version is ")(
                            m_strVersion)
                            .Flush();
                    }
                } else if (strNodeName->Compare("nymIDSource")) {
                    // noop
                } else if (strNodeName->Compare("inboxHashItem")) {
                    const auto strAccountID =
                        String::Factory(xml->getAttributeValue("accountID"));
                    const auto strHashValue =
                        String::Factory(xml->getAttributeValue("hashValue"));

                    LogDebug()(OT_PRETTY_CLASS())("InboxHash is ")(
                        strHashValue)(" for Account ID: ")(strAccountID)
                        .Flush();

                    // Make sure now that I've loaded this InboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID->Exists() && strHashValue->Exists()) {
                        auto pID = opentxs::Identifier::Factory(strHashValue);
                        OT_ASSERT(!pID->empty())
                        m_mapInboxHash.emplace(strAccountID->Get(), pID);
                    }
                } else if (strNodeName->Compare("outboxHashItem")) {
                    const auto strAccountID =
                        String::Factory(xml->getAttributeValue("accountID"));
                    const auto strHashValue =
                        String::Factory(xml->getAttributeValue("hashValue"));

                    LogDebug()(OT_PRETTY_CLASS())("OutboxHash is ")(
                        strHashValue)(" for Account ID: ")(strAccountID)
                        .Flush();

                    // Make sure now that I've loaded this OutboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID->Exists() && strHashValue->Exists()) {
                        OTIdentifier pID =
                            opentxs::Identifier::Factory(strHashValue);
                        OT_ASSERT(!pID->empty())
                        m_mapOutboxHash.emplace(strAccountID->Get(), pID);
                    }
                } else if (strNodeName->Compare("MARKED_FOR_DELETION")) {
                    m_bMarkForDeletion = true;
                    LogDebug()(OT_PRETTY_CLASS())(
                        "This nym has been MARKED_FOR_DELETION at some "
                        "point prior.")
                        .Flush();
                } else if (strNodeName->Compare("ownsAssetAcct")) {
                    auto strID = String::Factory(xml->getAttributeValue("ID"));

                    if (strID->Exists()) {
                        m_setAccounts.insert(strID->Get());
                        LogDebug()(OT_PRETTY_CLASS())(
                            "This nym has an asset account with the ID: ")(
                            strID)
                            .Flush();
                    } else
                        LogDebug()(OT_PRETTY_CLASS())(
                            "This nym MISSING asset account ID when loading "
                            "nym record.")
                            .Flush();
                } else if (strNodeName->Compare("outpaymentsMessage")) {
                    auto armorMail = Armored::Factory();
                    auto strMessage = String::Factory();

                    xml->read();

                    if (irr::io::EXN_TEXT == xml->getNodeType()) {
                        auto strNodeData = String::Factory(xml->getNodeData());

                        // Sometimes the XML reads up the data with a prepended
                        // newline.
                        // This screws up my own objects which expect a
                        // consistent
                        // in/out
                        // So I'm checking here for that prepended newline, and
                        // removing it.
                        char cNewline;
                        if (strNodeData->Exists() &&
                            strNodeData->GetLength() > 2 &&
                            strNodeData->At(0, cNewline)) {
                            if ('\n' == cNewline)
                                armorMail->Set(strNodeData->Get() + 1);
                            else
                                armorMail->Set(strNodeData->Get());

                            if (armorMail->GetLength() > 2) {
                                armorMail->GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage->GetLength() > 2) {
                                    auto pMessage = api_.Factory()
                                                        .InternalSession()
                                                        .Message();

                                    OT_ASSERT(false != bool(pMessage));

                                    if (pMessage->LoadContractFromString(
                                            strMessage)) {
                                        std::shared_ptr<Message> message{
                                            pMessage.release()};
                                        m_dequeOutpayments.push_back(message);
                                    }
                                }
                            }
                        }  // strNodeData
                    }      // EXN_TEXT
                }          // outpayments message
                else {
                    // unknown element type
                    LogError()(OT_PRETTY_CLASS())("Unknown element type in: ")(
                        xml->getNodeName())(".")
                        .Flush();
                    bSuccess = false;
                }
                break;
            }
            default: {
                LogInsane()(OT_PRETTY_CLASS())("Unknown XML type in ")(
                    xml->getNodeName())
                    .Flush();
                break;
            }
        }  // switch
    }      // while

    if (converted) { m_strVersion->Set("1.1"); }

    return bSuccess;
}

void NymFile::DisplayStatistics(opentxs::String& strOutput) const
{
    sLock lock(shared_lock_);

    auto out_payments = std::to_string(m_dequeOutpayments.size());
    auto source = target_nym_->Source().asString();
    auto alias = target_nym_->Alias();
    auto theStringID = String::Factory(target_nym_->ID());
    static std::string marked_for_deletion{"(MARKED FOR DELETION)"};

    static std::string fmt{
        "Source for ID:\n%s\nDescription: %s\n\n\n==>      Name: %s   %s\n     "
        " Version: %s\nOutpayments count: %s\nNym ID: %s\n"};
    UnallocatedVector<char> buf;
    buf.reserve(
        fmt.length() + 1 + source->GetLength() + m_strDescription->GetLength() +
        alias.length() + marked_for_deletion.length() +
        m_strVersion->GetLength() + out_payments.length() +
        theStringID->GetLength());

    auto size = std::snprintf(
        &buf[0],
        buf.capacity(),
        fmt.c_str(),
        source->Get(),
        m_strDescription->Get(),
        alias.c_str(),
        (m_bMarkForDeletion ? marked_for_deletion.c_str() : ""),
        m_strVersion->Get(),
        out_payments.c_str(),
        theStringID->Get());

    strOutput.Concatenate(String::Factory(&buf[0], size));
}

auto NymFile::GetHash(
    const mapOfIdentifiers& the_map,
    const UnallocatedCString& str_id,
    opentxs::Identifier& theOutput) const -> bool  // client-side
{
    sLock lock(shared_lock_);

    bool bRetVal =
        false;  // default is false: "No, I didn't find a hash for that id."
    theOutput.Release();

    // The Pseudonym has a map of its recent hashes, one for each server
    // (nymbox) or account (inbox/outbox).
    // For Server Bob, with this Pseudonym, I might have hash lkjsd987345lkj.
    // For but Server Alice, I might have hash 98345jkherkjghdf98gy.
    // (Same Nym, but different hash for each server, as well as inbox/outbox
    // hashes for each asset acct.)
    //
    // So let's loop through all the hashes I have, and if the ID on the map
    // passed in
    // matches the [server|acct] ID that was passed in, then return TRUE.
    //
    for (const auto& it : the_map) {
        if (str_id == it.first) {
            // The call has succeeded
            bRetVal = true;
            theOutput.SetString(it.second->str());
            break;
        }
    }

    return bRetVal;
}

auto NymFile::GetInboxHash(
    const UnallocatedCString& acct_id,
    opentxs::Identifier& theOutput) const -> bool  // client-side
{
    return GetHash(m_mapInboxHash, acct_id, theOutput);
}

auto NymFile::GetOutboxHash(
    const UnallocatedCString& acct_id,
    opentxs::Identifier& theOutput) const -> bool  // client-side
{
    return GetHash(m_mapOutboxHash, acct_id, theOutput);
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
auto NymFile::GetOutpaymentsByIndex(std::int32_t nIndex) const
    -> std::shared_ptr<Message>
{
    sLock lock(shared_lock_);
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size())) {

        return nullptr;
    }

    return m_dequeOutpayments.at(nIndex);
}

auto NymFile::GetOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    const PasswordPrompt& reason,
    std::unique_ptr<OTPayment>* pReturnPayment /*=nullptr*/,
    std::int32_t* pnReturnIndex /*=nullptr*/) const -> std::shared_ptr<Message>
{
    if (nullptr != pnReturnIndex) { *pnReturnIndex = -1; }

    const std::int32_t nCount = GetOutpaymentsCount();

    for (std::int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        auto pMsg = m_dequeOutpayments.at(nIndex);
        OT_ASSERT(false != bool(pMsg));
        auto strPayment = String::Factory();
        std::unique_ptr<OTPayment> payment;
        std::unique_ptr<OTPayment>& pPayment(
            nullptr == pReturnPayment ? payment : *pReturnPayment);

        // There isn't any encrypted envelope this time, since it's my
        // outPayments box.
        //
        if (pMsg->m_ascPayload->Exists() &&
            pMsg->m_ascPayload->GetString(strPayment) && strPayment->Exists()) {
            pPayment.reset(
                api_.Factory().InternalSession().Payment(strPayment).release());

            // Let's see if it's the cheque we're looking for...
            //
            if (pPayment && pPayment->IsValid()) {
                if (pPayment->SetTempValues(reason)) {
                    if (pPayment->HasTransactionNum(lTransNum, reason)) {

                        if (nullptr != pnReturnIndex) {
                            *pnReturnIndex = nIndex;
                        }

                        return pMsg;
                    }
                }
            }
        }
    }
    return nullptr;
}

/// return the number of payments items available for this Nym.
auto NymFile::GetOutpaymentsCount() const -> std::int32_t
{
    return static_cast<std::int32_t>(m_dequeOutpayments.size());
}

auto NymFile::LoadSignedNymFile(const PasswordPrompt& reason) -> bool
{
    sLock lock(shared_lock_);

    return load_signed_nymfile(lock, reason);
}

template <typename T>
auto NymFile::load_signed_nymfile(const T& lock, const PasswordPrompt& reason)
    -> bool
{
    OT_ASSERT(verify_lock(lock));

    // Get the Nym's ID in string form
    auto nymID = String::Factory(target_nym_->ID());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    auto theNymFile = api_.Factory().InternalSession().SignedFile(
        String::Factory(api_.Internal().Legacy().Nym()), nymID);

    if (!theNymFile->LoadFile()) {
        LogDetail()(OT_PRETTY_CLASS())("Failed loading a signed nymfile: ")(
            nymID)
            .Flush();

        return false;
    }

    // We verify:
    //
    // 1. That the file even exists and loads.
    // 2. That the local subdir and filename match the versions inside the file.
    // 3. That the signature matches for the signer nym who was passed in.
    //
    if (!theNymFile->VerifyFile()) {
        LogError()(OT_PRETTY_CLASS())("Failed verifying nymfile: ")(nymID)(".")
            .Flush();

        return false;
    }

    const auto& publicSignKey = signer_nym_->GetPublicSignKey();

    if (!theNymFile->VerifyWithKey(publicSignKey)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying signature on nymfile: ")(
            nymID)(". Signer Nym ID: ")(signer_nym_->ID())(".")
            .Flush();

        return false;
    }

    LogVerbose()(OT_PRETTY_CLASS())(
        "Loaded and verified signed nymfile. Reading from string... ")
        .Flush();

    if (1 > theNymFile->GetFilePayload().GetLength()) {
        const auto lLength = theNymFile->GetFilePayload().GetLength();

        LogError()(OT_PRETTY_CLASS())("Bad length (")(
            lLength)(") while loading nymfile: ")(nymID)(".")
            .Flush();
    }

    bool converted = false;
    const bool loaded = deserialize_nymfile(
        lock, theNymFile->GetFilePayload(), converted, nullptr);

    if (!loaded) { return false; }

    if (converted) {
        // This will ensure that none of the old tags will be present the next
        // time this nym is loaded.
        // Converting a nym more than once is likely to cause sync issues.
        save_signed_nymfile(lock, reason);
    }

    return true;
}

// Sometimes for testing I need to clear out all the transaction numbers from a
// nym. So I added this method to make such a thing easy to do.
void NymFile::RemoveAllNumbers(const opentxs::String& pstrNotaryID)
{
    UnallocatedList<mapOfIdentifiers::iterator> listOfInboxHash;
    UnallocatedList<mapOfIdentifiers::iterator> listOfOutboxHash;

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapInboxHash.begin()); it != m_mapInboxHash.end(); ++it) {
        listOfInboxHash.push_back(it);
    }

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapOutboxHash.begin()); it != m_mapOutboxHash.end(); ++it) {
        listOfOutboxHash.push_back(it);
    }

    while (!listOfInboxHash.empty()) {
        m_mapInboxHash.erase(listOfInboxHash.back());
        listOfInboxHash.pop_back();
    }
    while (!listOfOutboxHash.empty()) {
        m_mapOutboxHash.erase(listOfOutboxHash.back());
        listOfOutboxHash.pop_back();
    }
}

// if this function returns false, outpayments index was bad.
auto NymFile::RemoveOutpaymentsByIndex(const std::int32_t nIndex) -> bool
{
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size())) {
        LogError()(OT_PRETTY_CLASS())("Error: Index out of bounds: signed: ")(
            nIndex)(". Unsigned: ")(uIndex)(" (deque size is ")(
            m_dequeOutpayments.size())(").")
            .Flush();
        return false;
    }

    auto pMessage = m_dequeOutpayments.at(nIndex);
    OT_ASSERT(false != bool(pMessage));

    m_dequeOutpayments.erase(m_dequeOutpayments.begin() + uIndex);

    return true;
}

auto NymFile::RemoveOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    const PasswordPrompt& reason) -> bool
{
    std::int32_t nReturnIndex = -1;

    auto pMsg = this->GetOutpaymentsByTransNum(
        lTransNum, reason, nullptr, &nReturnIndex);
    const std::uint32_t uIndex = nReturnIndex;

    if ((nullptr != pMsg) && (nReturnIndex > (-1)) &&
        (uIndex < m_dequeOutpayments.size())) {
        m_dequeOutpayments.erase(m_dequeOutpayments.begin() + uIndex);
        return true;
    }
    return false;
}

// Save the Pseudonym to a string...
auto NymFile::SerializeNymFile(opentxs::String& output) const -> bool
{
    sLock lock(shared_lock_);

    return serialize_nymfile(lock, output);
}

template <typename T>
auto NymFile::serialize_nymfile(const T& lock, opentxs::String& strNym) const
    -> bool
{
    OT_ASSERT(verify_lock(lock));

    Tag tag("nymData");

    auto nymID = String::Factory(target_nym_->ID());

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("nymID", nymID->Get());

    if (m_lUsageCredits != 0)
        tag.add_attribute("usageCredits", std::to_string(m_lUsageCredits));

    target_nym_->SerializeNymIDSource(tag);

    // When you delete a Nym, it just marks it.
    // Actual deletion occurs during maintenance sweep
    // (targeting marked nyms...)
    //
    if (m_bMarkForDeletion) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS NYM HAS BEEN MARKED "
            "FOR DELETION AT ITS OWN REQUEST");
    }

    if (!(m_dequeOutpayments.empty())) {
        for (std::uint32_t i = 0; i < m_dequeOutpayments.size(); i++) {
            auto pMessage = m_dequeOutpayments.at(i);
            OT_ASSERT(false != bool(pMessage));

            auto strOutpayments = String::Factory(*pMessage);

            auto ascOutpayments = Armored::Factory();

            if (strOutpayments->Exists())
                ascOutpayments->SetString(strOutpayments);

            if (ascOutpayments->Exists()) {
                tag.add_tag("outpaymentsMessage", ascOutpayments->Get());
            }
        }
    }

    // These are used on the server side.
    // (That's why you don't see the server ID saved here.)
    //
    if (!(m_setAccounts.empty())) {
        for (auto& it : m_setAccounts) {
            UnallocatedCString strID(it);
            TagPtr pTag(new Tag("ownsAssetAcct"));
            pTag->add_attribute("ID", strID);
            tag.add_tag(pTag);
        }
    }

    // client-side
    for (auto& it : m_mapInboxHash) {
        UnallocatedCString strAcctID = it.first;
        const opentxs::Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const auto strHash = String::Factory(theID);
            TagPtr pTag(new Tag("inboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash->Get());
            tag.add_tag(pTag);
        }
    }  // for

    // client-side
    for (auto& it : m_mapOutboxHash) {
        UnallocatedCString strAcctID = it.first;
        const opentxs::Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const auto strHash = String::Factory(theID);
            TagPtr pTag(new Tag("outboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash->Get());
            tag.add_tag(pTag);
        }
    }  // for

    UnallocatedCString str_result;
    tag.output(str_result);

    strNym.Concatenate(String::Factory(str_result));

    return true;
}

auto NymFile::SerializeNymFile(const char* szFoldername, const char* szFilename)
    -> bool
{
    OT_ASSERT(nullptr != szFoldername);
    OT_ASSERT(nullptr != szFilename);

    sLock lock(shared_lock_);

    auto strNym = String::Factory();
    serialize_nymfile(lock, strNym);

    bool bSaved = OTDB::StorePlainString(
        api_,
        strNym->Get(),
        api_.DataFolder(),
        szFoldername,
        szFilename,
        "",
        "");
    if (!bSaved)
        LogError()(OT_PRETTY_CLASS())("Error saving file: ")(
            szFoldername)(api::Legacy::PathSeparator())(szFilename)(".")
            .Flush();

    return bSaved;
}

auto NymFile::SaveSignedNymFile(const PasswordPrompt& reason) -> bool
{
    eLock lock(shared_lock_);

    return save_signed_nymfile(lock, reason);
}

template <typename T>
auto NymFile::save_signed_nymfile(const T& lock, const PasswordPrompt& reason)
    -> bool
{
    OT_ASSERT(verify_lock(lock));

    // Get the Nym's ID in string form
    auto strNymID = String::Factory(target_nym_->ID());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    auto theNymFile = api_.Factory().InternalSession().SignedFile(
        api_.Internal().Legacy().Nym(), strNymID);
    theNymFile->GetFilename(m_strNymFile);

    LogVerbose()(OT_PRETTY_CLASS())("Saving nym to: ")(m_strNymFile).Flush();

    // First we save this nym to a string...
    // Specifically, the file payload string on the OTSignedFile object.
    serialize_nymfile(lock, theNymFile->GetFilePayload());

    // Now the OTSignedFile contains the path, the filename, AND the
    // contents of the Nym itself, saved to a string inside the OTSignedFile
    // object.

    const auto& privateSignKey = signer_nym_->GetPrivateSignKey();

    if (theNymFile->SignWithKey(privateSignKey, reason) &&
        theNymFile->SaveContract()) {
        const bool bSaved = theNymFile->SaveFile();

        if (!bSaved) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed while calling theNymFile->SaveFile() for Nym ")(
                strNymID)(" using Signer Nym ")(signer_nym_->ID())(".")
                .Flush();
        }

        return bSaved;
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed trying to sign and save NymFile for Nym ")(
            strNymID)(" using Signer Nym ")(signer_nym_->ID())(".")
            .Flush();
    }

    return false;
}

auto NymFile::SetHash(
    mapOfIdentifiers& the_map,
    const UnallocatedCString& str_id,
    const opentxs::Identifier& theInput) -> bool  // client-side
{
    the_map.emplace(str_id, theInput);

    return true;
}

auto NymFile::SetInboxHash(
    const UnallocatedCString& acct_id,
    const opentxs::Identifier& theInput) -> bool  // client-side
{
    eLock lock(shared_lock_);

    return SetHash(m_mapInboxHash, acct_id, theInput);
}

auto NymFile::SetOutboxHash(
    const UnallocatedCString& acct_id,
    const opentxs::Identifier& theInput) -> bool  // client-side
{
    eLock lock(shared_lock_);

    return SetHash(m_mapOutboxHash, acct_id, theInput);
}

NymFile::~NymFile() { ClearAll(); }
}  // namespace opentxs::implementation
