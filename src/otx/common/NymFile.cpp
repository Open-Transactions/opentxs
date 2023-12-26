// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/common/NymFile.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/identity/Source.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/crypto/OTSignedFile.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/internal.factory.hpp"
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
using namespace std::literals;

NymFile::NymFile(const api::Session& api, Nym_p targetNym, Nym_p signerNym)
    : api_{api}
    , target_nym_{targetNym}
    , signer_nym_{signerNym}
    , usage_credits_(0)
    , mark_for_deletion_(false)
    , nym_file_(String::Factory())
    , version_(String::Factory(NYMFILE_VERSION))
    , description_(String::Factory())
    , inbox_hash_()
    , outbox_hash_()
    , outpayments_()
    , accounts_()
{
}

/// a payments message is a form of transaction, transported via Nymbox
void NymFile::AddOutpayments(std::shared_ptr<Message> theMessage)
{
    const auto lock = eLock{shared_lock_};

    outpayments_.push_front(theMessage);
}

void NymFile::ClearAll()
{
    const auto lock = eLock{shared_lock_};

    inbox_hash_.clear();
    outbox_hash_.clear();
    accounts_.clear();
    outpayments_.clear();
}

auto NymFile::CompareID(const identifier::Nym& rhs) const -> bool
{
    auto lock = sLock{shared_lock_};

    return rhs == target_nym_->ID();
}

auto NymFile::DeserializeNymFile(
    const String& strNym,
    bool& converted,
    String::Map* pMapCredentials,
    const OTPassword* pImportPassword) -> bool
{
    auto lock = sLock{shared_lock_};

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
    assert_true(verify_lock(lock));

    bool bSuccess = false;
    bool convert = false;
    converted = false;
    //?    ClearAll();  // Since we are loading everything up... (credentials
    // are NOT cleared here. See note in Nym::ClearAll.)
    auto strNymXML = StringXML::Factory(strNym);  // todo optimize
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader(strNymXML.get());
    assert_false(nullptr == xml);
    const std::unique_ptr<irr::io::IrrXMLReader> theCleanup(xml);

    // parse the file until end reached
    while (xml && xml->read()) {

        // strings for storing the data that we want to read out of the file
        //
        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
            case irr::io::EXN_TEXT:
            case irr::io::EXN_COMMENT:
            case irr::io::EXN_ELEMENT_END:
            case irr::io::EXN_CDATA: {
            } break;
            case irr::io::EXN_ELEMENT: {
                const auto strNodeName = String::Factory(xml->getNodeName());

                if (strNodeName->Compare("nymData")) {
                    version_ =
                        String::Factory(xml->getAttributeValue("version"));
                    const auto UserNymID =
                        String::Factory(xml->getAttributeValue("nymID"));

                    // Server-side only...
                    auto strCredits =
                        String::Factory(xml->getAttributeValue("usageCredits"));

                    if (strCredits->GetLength() > 0) {
                        usage_credits_ = strCredits->ToLong();
                    } else {
                        usage_credits_ = 0;  // This is the default anyway, but
                    }
                    // just being safe...

                    if (UserNymID->GetLength()) {
                        LogDebug()()("Loading user, version: ")(version_.get())(
                            " NymID: ")(UserNymID.get())
                            .Flush();
                    }
                    bSuccess = true;
                    convert = (String::Factory("1.0")->Compare(version_));

                    if (convert) {
                        LogError()()("Converting nymfile with version ")(
                            version_.get())(".")
                            .Flush();
                    } else {
                        LogDetail()()(
                            "Not converting nymfile because version is ")(
                            version_.get())
                            .Flush();
                    }
                } else if (strNodeName->Compare("nymIDSource")) {
                    // noop
                } else if (strNodeName->Compare("inboxHashItem")) {
                    const auto strAccountID =
                        String::Factory(xml->getAttributeValue("accountID"));
                    const auto strHashValue =
                        String::Factory(xml->getAttributeValue("hashValue"));

                    LogDebug()()("InboxHash is ")(strHashValue.get())(
                        " for Account ID: ")(strAccountID.get())
                        .Flush();

                    // Make sure now that I've loaded this InboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID->Exists() && strHashValue->Exists()) {
                        auto pID = api_.Factory().IdentifierFromBase58(
                            strHashValue->Bytes());
                        assert_false(pID.empty());
                        inbox_hash_.emplace(strAccountID->Get(), pID);
                    }
                } else if (strNodeName->Compare("outboxHashItem")) {
                    const auto strAccountID =
                        String::Factory(xml->getAttributeValue("accountID"));
                    const auto strHashValue =
                        String::Factory(xml->getAttributeValue("hashValue"));

                    LogDebug()()("OutboxHash is ")(strHashValue.get())(
                        " for Account ID: ")(strAccountID.get())
                        .Flush();

                    // Make sure now that I've loaded this OutboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID->Exists() && strHashValue->Exists()) {
                        identifier::Generic pID =
                            api_.Factory().IdentifierFromBase58(
                                strHashValue->Bytes());
                        assert_false(pID.empty());
                        outbox_hash_.emplace(strAccountID->Get(), pID);
                    }
                } else if (strNodeName->Compare("MARKED_FOR_DELETION")) {
                    mark_for_deletion_ = true;
                    LogDebug()()(
                        "This nym has been MARKED_FOR_DELETION at some "
                        "point prior.")
                        .Flush();
                } else if (strNodeName->Compare("ownsAssetAcct")) {
                    auto strID = String::Factory(xml->getAttributeValue("ID"));

                    if (strID->Exists()) {
                        accounts_.insert(strID->Get());
                        LogDebug()()(
                            "This nym has an asset account with the ID: ")(
                            strID.get())
                            .Flush();
                    } else {
                        LogDebug()()(
                            "This nym MISSING asset account ID when loading "
                            "nym record.")
                            .Flush();
                    }
                } else if (strNodeName->Compare("outpaymentsMessage")) {
                    auto armorMail = Armored::Factory(api_.Crypto());
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
                            if ('\n' == cNewline) {
                                armorMail->Set(strNodeData->Get() + 1);
                            } else {
                                armorMail->Set(strNodeData->Get());
                            }

                            if (armorMail->GetLength() > 2) {
                                armorMail->GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage->GetLength() > 2) {
                                    auto pMessage = api_.Factory()
                                                        .Internal()
                                                        .Session()
                                                        .Message();

                                    assert_true(false != bool(pMessage));

                                    if (pMessage->LoadContractFromString(
                                            strMessage)) {
                                        const std::shared_ptr<Message> message{
                                            pMessage.release()};
                                        outpayments_.push_back(message);
                                    }
                                }
                            }
                        }  // strNodeData
                    }      // EXN_TEXT
                }          // outpayments message
                else {
                    // unknown element type
                    LogError()()("Unknown element type in: ")(
                        xml->getNodeName())(".")
                        .Flush();
                    bSuccess = false;
                }
                break;
            }
            case irr::io::EXN_UNKNOWN:
            default: {
                LogInsane()()("Unknown XML type in ")(xml->getNodeName())
                    .Flush();
            }
        }  // switch
    }      // while

    if (converted) { version_->Set("1.1"); }

    return bSuccess;
}

void NymFile::DisplayStatistics(opentxs::String& strOutput) const
{
    auto lock = sLock{shared_lock_};
    const auto marked_for_deletion = [&]() -> std::string_view {
        if (mark_for_deletion_) {

            return "(MARKED FOR DELETION)"sv;
        } else {

            return {};
        }
    }();
    strOutput.Concatenate("Source for ID:\n"sv)
        .Concatenate(target_nym_->Source().Internal().asString())
        .Concatenate("\nDescription: "sv)
        .Concatenate(description_)
        .Concatenate("\n\n\n==>      Name: "sv)
        .Concatenate(target_nym_->Alias())
        .Concatenate("   "sv)
        .Concatenate(marked_for_deletion)
        .Concatenate("\n      Version: "sv)
        .Concatenate(version_)
        .Concatenate("\nOutpayments count: "sv)
        .Concatenate(std::to_string(outpayments_.size()))
        .Concatenate("\nNym ID: "sv)
        .Concatenate(target_nym_->ID().asBase58(api_.Crypto()))
        .Concatenate("\n"sv);
}

auto NymFile::GetHash(
    const mapOfIdentifiers& the_map,
    const UnallocatedCString& str_id,
    opentxs::identifier::Generic& theOutput) const -> bool  // client-side
{
    auto lock = sLock{shared_lock_};

    bool bRetVal =
        false;  // default is false: "No, I didn't find a hash for that id."
    theOutput.clear();

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
            theOutput = it.second;
            break;
        }
    }

    return bRetVal;
}

auto NymFile::GetInboxHash(
    const UnallocatedCString& acct_id,
    opentxs::identifier::Generic& theOutput) const -> bool  // client-side
{
    return GetHash(inbox_hash_, acct_id, theOutput);
}

auto NymFile::GetOutboxHash(
    const UnallocatedCString& acct_id,
    opentxs::identifier::Generic& theOutput) const -> bool  // client-side
{
    return GetHash(outbox_hash_, acct_id, theOutput);
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
auto NymFile::GetOutpaymentsByIndex(std::int32_t nIndex) const
    -> std::shared_ptr<Message>
{
    auto lock = sLock{shared_lock_};
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (outpayments_.empty() || (nIndex < 0) ||
        (uIndex >= outpayments_.size())) {

        return nullptr;
    }

    return outpayments_.at(nIndex);
}

auto NymFile::GetOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    const opentxs::PasswordPrompt& reason,
    std::unique_ptr<OTPayment>* pReturnPayment /*=nullptr*/,
    std::int32_t* pnReturnIndex /*=nullptr*/) const -> std::shared_ptr<Message>
{
    if (nullptr != pnReturnIndex) { *pnReturnIndex = -1; }

    const std::int32_t nCount = GetOutpaymentsCount();

    for (std::int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        auto pMsg = outpayments_.at(nIndex);
        assert_true(false != bool(pMsg));
        auto strPayment = String::Factory();
        std::unique_ptr<OTPayment> payment;
        std::unique_ptr<OTPayment>& pPayment(
            nullptr == pReturnPayment ? payment : *pReturnPayment);

        // There isn't any encrypted envelope this time, since it's my
        // outPayments box.
        //
        if (pMsg->payload_->Exists() && pMsg->payload_->GetString(strPayment) &&
            strPayment->Exists()) {
            pPayment = api_.Factory().Internal().Session().Payment(strPayment);

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
    return static_cast<std::int32_t>(outpayments_.size());
}

auto NymFile::LoadSignedNymFile(const opentxs::PasswordPrompt& reason) -> bool
{
    auto lock = sLock{shared_lock_};

    return load_signed_nymfile(lock, reason);
}

template <typename T>
auto NymFile::load_signed_nymfile(
    const T& lock,
    const opentxs::PasswordPrompt& reason) -> bool
{
    assert_true(verify_lock(lock));

    // Get the Nym's ID in string form
    auto nymID = String::Factory(target_nym_->ID(), api_.Crypto());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    auto theNymFile = api_.Factory().Internal().Session().SignedFile(
        String::Factory(api_.Internal().Paths().Nym()), nymID);

    if (!theNymFile->LoadFile()) {
        LogDetail()()("Failed loading a signed nymfile: ")(nymID.get()).Flush();

        return false;
    }

    // We verify:
    //
    // 1. That the file even exists and loads.
    // 2. That the local subdir and filename match the versions inside the file.
    // 3. That the signature matches for the signer nym who was passed in.
    //
    if (!theNymFile->VerifyFile()) {
        LogError()()("Failed verifying nymfile: ")(nymID.get())(".").Flush();

        return false;
    }

    const auto& publicSignKey = signer_nym_->GetPublicSignKey();

    if (!theNymFile->VerifyWithKey(publicSignKey)) {
        LogError()()("Failed verifying signature on nymfile: ")(nymID.get())(
            ". Signer Nym ID: ")(signer_nym_->ID(), api_.Crypto())(".")
            .Flush();

        return false;
    }

    LogVerbose()()(
        "Loaded and verified signed nymfile. Reading from string... ")
        .Flush();

    if (1 > theNymFile->GetFilePayload().GetLength()) {
        const auto lLength = theNymFile->GetFilePayload().GetLength();

        LogError()()("Bad length (")(lLength)(") while loading nymfile: ")(
            nymID.get())(".")
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
    for (auto it(inbox_hash_.begin()); it != inbox_hash_.end(); ++it) {
        listOfInboxHash.push_back(it);
    }

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(outbox_hash_.begin()); it != outbox_hash_.end(); ++it) {
        listOfOutboxHash.push_back(it);
    }

    while (!listOfInboxHash.empty()) {
        inbox_hash_.erase(listOfInboxHash.back());
        listOfInboxHash.pop_back();
    }
    while (!listOfOutboxHash.empty()) {
        outbox_hash_.erase(listOfOutboxHash.back());
        listOfOutboxHash.pop_back();
    }
}

// if this function returns false, outpayments index was bad.
auto NymFile::RemoveOutpaymentsByIndex(const std::int32_t nIndex) -> bool
{
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (outpayments_.empty() || (nIndex < 0) ||
        (uIndex >= outpayments_.size())) {
        LogError()()("Error: Index out of bounds: signed: ")(
            nIndex)(". Unsigned: ")(uIndex)(" (deque size is ")(
            outpayments_.size())(").")
            .Flush();
        return false;
    }

    auto pMessage = outpayments_.at(nIndex);
    assert_true(false != bool(pMessage));

    outpayments_.erase(outpayments_.begin() + uIndex);

    return true;
}

auto NymFile::RemoveOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    const opentxs::PasswordPrompt& reason) -> bool
{
    std::int32_t nReturnIndex = -1;

    auto pMsg = this->GetOutpaymentsByTransNum(
        lTransNum, reason, nullptr, &nReturnIndex);
    const std::uint32_t uIndex = nReturnIndex;

    if ((nullptr != pMsg) && (nReturnIndex > (-1)) &&
        (uIndex < outpayments_.size())) {
        outpayments_.erase(outpayments_.begin() + uIndex);
        return true;
    }
    return false;
}

// Save the Pseudonym to a string...
auto NymFile::SerializeNymFile(opentxs::String& output) const -> bool
{
    auto lock = sLock{shared_lock_};

    return serialize_nymfile(lock, output);
}

template <typename T>
auto NymFile::serialize_nymfile(const T& lock, opentxs::String& strNym) const
    -> bool
{
    assert_true(verify_lock(lock));

    Tag tag("nymData");

    auto nymID = String::Factory(target_nym_->ID(), api_.Crypto());

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("nymID", nymID->Get());

    if (usage_credits_ != 0) {
        tag.add_attribute("usageCredits", std::to_string(usage_credits_));
    }

    target_nym_->SerializeNymIDSource(tag);

    // When you delete a Nym, it just marks it.
    // Actual deletion occurs during maintenance sweep
    // (targeting marked nyms...)
    //
    if (mark_for_deletion_) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS NYM HAS BEEN MARKED "
            "FOR DELETION AT ITS OWN REQUEST");
    }

    if (!(outpayments_.empty())) {
        for (auto pMessage : outpayments_) {
            assert_true(false != bool(pMessage));

            auto strOutpayments = String::Factory(*pMessage);

            auto ascOutpayments = Armored::Factory(api_.Crypto());

            if (strOutpayments->Exists()) {
                ascOutpayments->SetString(strOutpayments);
            }

            if (ascOutpayments->Exists()) {
                tag.add_tag("outpaymentsMessage", ascOutpayments->Get());
            }
        }
    }

    // These are used on the server side.
    // (That's why you don't see the server ID saved here.)
    //
    if (!(accounts_.empty())) {
        for (const auto& it : accounts_) {
            const UnallocatedCString strID(it);
            TagPtr pTag(new Tag("ownsAssetAcct"));
            pTag->add_attribute("ID", strID);
            tag.add_tag(pTag);
        }
    }

    // client-side
    for (const auto& it : inbox_hash_) {
        const UnallocatedCString strAcctID = it.first;
        const identifier::Generic& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const auto strHash = String::Factory(theID, api_.Crypto());
            TagPtr pTag(new Tag("inboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash->Get());
            tag.add_tag(pTag);
        }
    }  // for

    // client-side
    for (const auto& it : outbox_hash_) {
        const UnallocatedCString strAcctID = it.first;
        const identifier::Generic& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const auto strHash = String::Factory(theID, api_.Crypto());
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
    assert_false(nullptr == szFoldername);
    assert_false(nullptr == szFilename);

    auto lock = sLock{shared_lock_};

    auto strNym = String::Factory();
    serialize_nymfile(lock, strNym);

    const bool bSaved = OTDB::StorePlainString(
        api_,
        strNym->Get(),
        api_.DataFolder().string(),
        szFoldername,
        szFilename,
        "",
        "");
    if (!bSaved) {
        LogError()()("Error saving file: ")(szFoldername)('/')(szFilename)(".")
            .Flush();
    }

    return bSaved;
}

auto NymFile::SaveSignedNymFile(const opentxs::PasswordPrompt& reason) -> bool
{
    const auto lock = eLock{shared_lock_};

    return save_signed_nymfile(lock, reason);
}

template <typename T>
auto NymFile::save_signed_nymfile(
    const T& lock,
    const opentxs::PasswordPrompt& reason) -> bool
{
    assert_true(verify_lock(lock));

    // Get the Nym's ID in string form
    auto strNymID = String::Factory(target_nym_->ID(), api_.Crypto());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    auto theNymFile = api_.Factory().Internal().Session().SignedFile(
        api_.Internal().Paths().Nym(), strNymID);
    theNymFile->GetFilename(nym_file_);

    LogVerbose()()("Saving nym to: ")(nym_file_.get()).Flush();

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
            LogError()()(
                "Failed while calling theNymFile->SaveFile() for Nym ")(
                strNymID.get())(" using Signer Nym ")(
                signer_nym_->ID(), api_.Crypto())(".")
                .Flush();
        }

        return bSaved;
    } else {
        LogError()()("Failed trying to sign and save NymFile for Nym ")(
            strNymID.get())(" using Signer Nym ")(
            signer_nym_->ID(), api_.Crypto())(".")
            .Flush();
    }

    return false;
}

auto NymFile::SetHash(
    mapOfIdentifiers& the_map,
    const UnallocatedCString& str_id,
    const identifier::Generic& theInput) -> bool  // client-side
{
    the_map.emplace(str_id, theInput);

    return true;
}

auto NymFile::SetInboxHash(
    const UnallocatedCString& acct_id,
    const identifier::Generic& theInput) -> bool  // client-side
{
    const auto lock = eLock{shared_lock_};

    return SetHash(inbox_hash_, acct_id, theInput);
}

auto NymFile::SetOutboxHash(
    const UnallocatedCString& acct_id,
    const identifier::Generic& theInput) -> bool  // client-side
{
    const auto lock = eLock{shared_lock_};

    return SetHash(outbox_hash_, acct_id, theInput);
}

NymFile::~NymFile() { ClearAll(); }
}  // namespace opentxs::implementation
