// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Nym.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <irrxml/irrXML.hpp>
#include <sodium/crypto_box.h>
#include <sys/types.h>

#include <array>
#include <fstream>
#include <functional>
#include <string>

#define NYMFILE_VERSION "1.1"

#define OT_METHOD "opentxs::Nym::"

namespace opentxs
{
Nym::Nym(
    const api::Core& api,
    const Identifier& nymID,
    const proto::CredentialIndexMode mode)
    : api_(api)
    , version_(NYM_CREATE_VERSION)
    , index_(0)
    , alias_()
    , revision_(1)
    , mode_(mode)
    , m_strVersion(String::Factory(NYMFILE_VERSION))
    , m_strDescription(String::Factory())
    , m_nymID(Identifier::Factory(nymID))
    , source_(nullptr)
    , contact_data_(nullptr)
    , m_mapCredentialSets()
    , m_mapRevokedSets()
    , m_listRevokedIDs()
{
}

Nym::Nym(const api::Core& api, const NymParameters& nymParameters)
    : Nym(api, Identifier::Factory(), proto::CREDINDEX_PRIVATE)
{
    NymParameters revisedParameters = nymParameters;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    revisedParameters.SetCredset(index_++);
    std::uint32_t nymIndex = 0;
    std::string fingerprint = nymParameters.Seed();
    auto seed = api_.Seeds().Seed(fingerprint, nymIndex);

    OT_ASSERT(seed);

    const bool defaultIndex = nymParameters.UseAutoIndex();

    if (!defaultIndex) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Re-creating nym at specified path.")
            .Flush();

        nymIndex = nymParameters.Nym();
    }

    const std::int32_t newIndex = nymIndex + 1;

    api_.Seeds().UpdateIndex(fingerprint, newIndex);
    revisedParameters.SetEntropy(*seed);
    revisedParameters.SetSeed(fingerprint);
    revisedParameters.SetNym(nymIndex);
#endif
    CredentialSet* pNewCredentialSet =
        new CredentialSet(api_, revisedParameters, version_);

    OT_ASSERT(nullptr != pNewCredentialSet);

    source_ = std::make_shared<NymIDSource>(pNewCredentialSet->Source());
    const_cast<OTIdentifier&>(m_nymID) = source_->NymID();

    SetDescription(source_->Description());

    m_mapCredentialSets.insert(std::pair<std::string, CredentialSet*>(
        pNewCredentialSet->GetMasterCredID(), pNewCredentialSet));
}

bool Nym::add_contact_credential(
    const eLock& lock,
    const proto::ContactData& data)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddContactCredential(data);

                break;
            }
        }
    }

    return added;
}

bool Nym::add_verification_credential(
    const eLock& lock,
    const proto::VerificationSet& data)
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddVerificationCredential(data);

                break;
            }
        }
    }

    return added;
}

std::string Nym::AddChildKeyCredential(
    const Identifier& masterID,
    const NymParameters& nymParameters)
{
    eLock lock(shared_lock_);

    std::string output;
    std::string master = masterID.str();
    auto it = m_mapCredentialSets.find(master);
    const bool noMaster = (it == m_mapCredentialSets.end());

    if (noMaster) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Master ID not found.").Flush();

        return output;
    }

    if (it->second) {
        output = it->second->AddChildKeyCredential(nymParameters);
    }

    return output;
}

bool Nym::AddClaim(const Claim& claim)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->AddItem(claim)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddContract(
    const Identifier& instrumentDefinitionID,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active)
{
    const std::string id(instrumentDefinitionID.str());

    if (id.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddContract(id, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
bool Nym::AddPaymentCode(
    const class PaymentCode& code,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active)
{
    const auto paymentCode = code.asBase58();

    if (paymentCode.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddPaymentCode(paymentCode, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}
#endif

bool Nym::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(
        new ContactData(contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddPreferredOTServer(const Identifier& id, const bool primary)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_)

    contact_data_.reset(
        new ContactData(contact_data_->AddPreferredOTServer(id, primary)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active)
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

std::string Nym::Alias() const { return alias_; }

const serializedCredentialIndex Nym::asPublicNym() const
{
    return SerializeCredentialIndex(CREDENTIAL_INDEX_MODE_FULL_CREDS);
}

std::shared_ptr<const proto::Credential> Nym::ChildCredentialContents(
    const std::string& masterID,
    const std::string& childID) const
{
    sLock lock(shared_lock_);

    std::shared_ptr<const proto::Credential> output;
    auto credential = MasterCredential(String::Factory(masterID));

    if (nullptr != credential) {
        output = credential->GetChildCredential(String::Factory(childID))
                     ->Serialized(AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

std::string Nym::BestEmail() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestEmail();
}

std::string Nym::BestPhoneNumber() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestPhoneNumber();
}

std::string Nym::BestSocialMediaProfile(const proto::ContactItemType type) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestSocialMediaProfile(type);
}

const class ContactData& Nym::Claims() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return *contact_data_;
}

void Nym::ClearCredentials()
{
    eLock lock(shared_lock_);

    clear_credentials(lock);
}

void Nym::clear_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    m_listRevokedIDs.clear();

    while (!m_mapCredentialSets.empty()) {
        CredentialSet* pCredential = m_mapCredentialSets.begin()->second;
        m_mapCredentialSets.erase(m_mapCredentialSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }

    while (!m_mapRevokedSets.empty()) {
        CredentialSet* pCredential = m_mapRevokedSets.begin()->second;
        m_mapRevokedSets.erase(m_mapRevokedSets.begin());
        delete pCredential;
        pCredential = nullptr;
    }
}

bool Nym::CompareID(const Nym& rhs) const
{
    sLock lock(shared_lock_);

    return rhs.CompareID(m_nymID);
}

bool Nym::CompareID(const Identifier& rhs) const
{
    sLock lock(shared_lock_);

    return rhs == m_nymID;
}

std::set<OTIdentifier> Nym::Contracts(
    const proto::ContactItemType currency,
    const bool onlyActive) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->Contracts(currency, onlyActive);
}

bool Nym::DeleteClaim(const Identifier& id)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->Delete(id)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

void Nym::DisplayStatistics(String& strOutput) const
{
    sLock lock(shared_lock_);
    strOutput.Concatenate("Source for ID:\n%s\n", Source().asString()->Get());
    strOutput.Concatenate("Description: %s\n\n", m_strDescription->Get());
    strOutput.Concatenate("%s", "\n");
    strOutput.Concatenate("==>      Name: %s\n", Alias().c_str());
    strOutput.Concatenate("      Version: %s\n", m_strVersion->Get());
    auto theStringID = String::Factory(ID());
    strOutput.Concatenate("Nym ID: %s\n", theStringID->Get());
}

std::string Nym::EmailAddresses(bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->EmailAddresses(active);
}

const std::vector<OTIdentifier> Nym::GetChildCredentialIDs(
    const std::string& masterID) const
{
    sLock lock(shared_lock_);

    std::vector<OTIdentifier> ids;

    auto it = m_mapCredentialSets.find(masterID);
    if (m_mapCredentialSets.end() != it) {
        const CredentialSet* pMaster = it->second;

        OT_ASSERT(nullptr != pMaster);

        auto count = pMaster->GetChildCredentialCount();
        for (size_t i = 0; i < count; ++i) {
            auto pChild = pMaster->GetChildCredentialByIndex(i);

            OT_ASSERT(nullptr != pChild);

            ids.emplace_back(pChild->ID());
        }
    }

    return ids;
}

void Nym::GetIdentifier(Identifier& theIdentifier) const
{
    sLock lock(shared_lock_);

    theIdentifier.Assign(m_nymID);
}

// sets argument based on internal member
void Nym::GetIdentifier(String& theIdentifier) const
{
    sLock lock(shared_lock_);

    m_nymID->GetString(theIdentifier);
}

CredentialSet* Nym::GetMasterCredential(const String& strID)
{
    sLock lock(shared_lock_);
    auto iter = m_mapCredentialSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

const std::vector<OTIdentifier> Nym::GetMasterCredentialIDs() const
{
    sLock lock(shared_lock_);

    std::vector<OTIdentifier> ids;

    for (const auto& it : m_mapCredentialSets) {
        const CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        ids.emplace_back(pCredential->GetMasterCredential().ID());
    }

    return ids;
}

const crypto::key::Asymmetric& Nym::GetPrivateAuthKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_private_auth_key(lock, keytype);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_private_auth_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));
    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateAuthKey(
        keytype, &m_listRevokedIDs);  // success
}

void Nym::GetPrivateCredentials(String& strCredList, String::Map* pmapCredFiles)
    const
{
    Tag tag("nymData");

    tag.add_attribute("version", m_strVersion->Get());

    auto strNymID = String::Factory(m_nymID);

    tag.add_attribute("nymID", strNymID->Get());

    SerializeNymIDSource(tag);

    SaveCredentialsToTag(tag, nullptr, pmapCredFiles);

    std::string str_result;
    tag.output(str_result);

    strCredList.Concatenate("%s", str_result.c_str());
}

const crypto::key::Asymmetric& Nym::GetPrivateEncrKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateEncrKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPrivateSignKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_private_sign_key(lock, keytype);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_private_sign_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPrivateSignKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPublicAuthKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicAuthKey(
        keytype,
        &m_listRevokedIDs);  // success
}

const crypto::key::Asymmetric& Nym::GetPublicEncrKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    OT_ASSERT(!m_mapCredentialSets.empty());

    const CredentialSet* pCredential = nullptr;
    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicEncrKey(
        keytype,
        &m_listRevokedIDs);  // success
}

// This is being called by:
// Contract::VerifySignature(const Nym& theNym, const Signature&
// theSignature, OTPasswordData * pPWData=nullptr)
//
// Note: Need to change Contract::VerifySignature so that it checks all of
// these keys when verifying.
//
// OT uses the signature's metadata to narrow down its search for the correct
// public key.
// Return value is the count of public keys found that matched the metadata on
// the signature.
std::int32_t Nym::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
    char cKeyType) const
{
    // Unfortunately, theSignature can only narrow the search down (there may be
    // multiple results.)
    std::int32_t nCount = 0;

    sLock lock(shared_lock_);

    for (const auto& it : m_mapCredentialSets) {
        const CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        const std::int32_t nTempCount = pCredential->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }

    return nCount;
}

const crypto::key::Asymmetric& Nym::GetPublicSignKey(
    proto::AsymmetricKeyType keytype) const
{
    sLock lock(shared_lock_);

    return get_public_sign_key(lock, keytype);
}

template <typename T>
const crypto::key::Asymmetric& Nym::get_public_sign_key(
    const T& lock,
    proto::AsymmetricKeyType keytype) const
{
    OT_ASSERT(!m_mapCredentialSets.empty());

    OT_ASSERT(verify_lock(lock));

    const CredentialSet* pCredential = nullptr;

    for (const auto& it : m_mapCredentialSets) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second;
        if (nullptr != pCredential) break;
    }
    if (nullptr == pCredential) OT_FAIL;

    return pCredential->GetPublicSignKey(
        keytype,
        &m_listRevokedIDs);  // success
}

CredentialSet* Nym::GetRevokedCredential(const String& strID)
{
    sLock lock(shared_lock_);

    auto iter = m_mapRevokedSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

const std::vector<OTIdentifier> Nym::GetRevokedCredentialIDs() const
{
    sLock lock(shared_lock_);

    std::vector<OTIdentifier> ids;

    for (const auto& it : m_mapRevokedSets) {
        const CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        ids.emplace_back(pCredential->GetMasterCredential().ID());
    }

    return ids;
}

bool Nym::HasCapability(const NymCapability& capability) const
{
    eLock lock(shared_lock_);

    return has_capability(lock, capability);
}

bool Nym::has_capability(const eLock& lock, const NymCapability& capability)
    const
{
    OT_ASSERT(verify_lock(lock));

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const CredentialSet& credSet = *it.second;

            if (credSet.hasCapability(capability)) { return true; }
        }
    }

    return false;
}

void Nym::init_claims(const eLock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    const auto nymID{m_nymID->str()};
    // const std::string nymID = String::Factory(m_nymID)->Get();

    contact_data_.reset(new class ContactData(
        nymID,
        NYM_CONTACT_DATA_VERSION,
        NYM_CONTACT_DATA_VERSION,
        ContactData::SectionMap()));

    OT_ASSERT(contact_data_);

    std::unique_ptr<proto::ContactData> serialized{nullptr};

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        const auto& credSet = *it.second;
        credSet.GetContactData(serialized);

        if (serialized) {
            OT_ASSERT(
                proto::Validate(*serialized, VERBOSE, proto::CLAIMS_NORMAL));

            class ContactData claimCred(
                nymID, NYM_CONTACT_DATA_VERSION, *serialized);
            contact_data_.reset(
                new class ContactData(*contact_data_ + claimCred));
            serialized.reset();
        }
    }

    OT_ASSERT(contact_data_)
}

bool Nym::LoadCredentialIndex(const serializedCredentialIndex& index)
{
    eLock lock(shared_lock_);

    return load_credential_index(lock, index);
}

bool Nym::load_credential_index(
    const eLock& lock,
    const serializedCredentialIndex& index)
{
    if (!proto::Validate<proto::CredentialIndex>(index, VERBOSE)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load invalid serialized"
                                           " credential index.")
            .Flush();

        return false;
    }

    OT_ASSERT(verify_lock(lock));

    const auto nymID = Identifier::Factory(index.nymid());

    if (m_nymID != nymID) { return false; }

    version_ = index.version();
    index_ = index.index();
    revision_.store(index.revision());
    mode_ = index.mode();
    source_ = std::make_shared<NymIDSource>(api_.Factory(), index.source());
    proto::KeyMode mode = (proto::CREDINDEX_PRIVATE == mode_)
                              ? proto::KEYMODE_PRIVATE
                              : proto::KEYMODE_PUBLIC;
    contact_data_.reset();
    m_mapCredentialSets.clear();

    for (auto& it : index.activecredentials()) {
        CredentialSet* newSet = new CredentialSet(api_, mode, it);

        if (nullptr != newSet) {
            m_mapCredentialSets.emplace(
                std::make_pair(newSet->GetMasterCredID(), newSet));
        }
    }

    m_mapRevokedSets.clear();

    for (auto& it : index.revokedcredentials()) {
        CredentialSet* newSet = new CredentialSet(api_, mode, it);

        if (nullptr != newSet) {
            m_mapRevokedSets.emplace(
                std::make_pair(newSet->GetMasterCredID(), newSet));
        }
    }

    return true;
}

// Use this to load the keys for a Nym (whether public or private), and then
// call VerifyPseudonym, and then load the actual Nymfile using
// LoadSignedNymfile.
bool Nym::LoadCredentials(
    bool bLoadPrivate,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    eLock lock(shared_lock_);

    return load_credentials(lock);
}

bool Nym::load_credentials(
    const eLock& lock,
    bool bLoadPrivate,
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword)
{
    clear_credentials(lock);

    auto strNymID = String::Factory(m_nymID);
    std::shared_ptr<proto::CredentialIndex> index;

    if (api_.Storage().Load(strNymID->Get(), index)) {
        return load_credential_index(lock, *index);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to load credential list for nym: ")(strNymID)(".")
            .Flush();
    }

    return false;
}

// This version is run on the server side, and assumes only a Public Key.
// This code reads up the file, discards the bookends, and saves only the
// gibberish itself.
bool Nym::LoadPublicKey()
{
    eLock lock(shared_lock_);

    if (load_credentials(lock) && (m_mapCredentialSets.size() > 0)) {
        return true;
    }
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Failure. ").Flush();
    return false;
}

const CredentialSet* Nym::MasterCredential(const String& strID) const
{
    auto iter = m_mapCredentialSets.find(strID.Get());
    CredentialSet* pCredential = nullptr;

    if (m_mapCredentialSets.end() != iter)  // found it
        pCredential = iter->second;

    return pCredential;
}

std::shared_ptr<const proto::Credential> Nym::MasterCredentialContents(
    const std::string& id) const
{
    eLock lock(shared_lock_);

    std::shared_ptr<const proto::Credential> output;
    auto temp = String::Factory(id);
    auto credential = MasterCredential(temp);

    if (nullptr != credential) {
        output = credential->GetMasterCredential().Serialized(
            AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

std::string Nym::Name() const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    std::string output = contact_data_->Name();

    if (false == output.empty()) { return output; }

    return alias_;
}

bool Nym::Path(proto::HDPath& output) const
{
    sLock lock(shared_lock_);

    for (const auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);
        const auto& set = *it.second;

        if (set.Path(output)) {
            output.mutable_child()->RemoveLast();
            return true;
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": No credential set contains a path.")
        .Flush();

    return false;
}

std::string Nym::PaymentCode() const
{
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    if (!source_) { return ""; }

    if (proto::SOURCETYPE_BIP47 != source_->Type()) { return ""; }

    auto serialized = source_->Serialize();

    if (!serialized) { return ""; }

    auto paymentCode = api_.Factory().PaymentCode(serialized->paymentcode());

    return paymentCode->asBase58();

#else
    return "";
#endif
}

std::string Nym::PhoneNumbers(bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->PhoneNumbers(active);
}

// Used when importing/exporting Nym into and out-of the sphere of the
// cached key in the wallet.
bool Nym::ReEncryptPrivateCredentials(
    bool bImporting,  // bImporting=true, or
                      // false if exporting.
    const OTPasswordData* pPWData,
    const OTPassword* pImportPassword) const
{
    eLock lock(shared_lock_);

    const OTPassword* pExportPassphrase = nullptr;
    std::unique_ptr<const OTPassword> thePasswordAngel;

    if (nullptr == pImportPassword) {

        // whether import/export, this display string is for the OUTSIDE OF
        // WALLET
        // portion of that process.
        //
        auto strDisplay = String::Factory(
            nullptr != pPWData
                ? pPWData->GetDisplayString()
                : (bImporting ? "Enter passphrase for the Nym being imported."
                              : "Enter passphrase for exported Nym."));
        // Circumvents the cached key.
        pExportPassphrase = crypto::key::LegacySymmetric::GetPassphraseFromUser(
            strDisplay, !bImporting);  // bAskTwice is true when exporting
                                       // (since the export passphrase is being
                                       // created at that time.)
        thePasswordAngel.reset(pExportPassphrase);

        if (nullptr == pExportPassphrase) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed in GetPassphraseFromUser.")
                .Flush();
            return false;
        }
    } else {
        pExportPassphrase = pImportPassword;
    }

    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        if (false == pCredential->ReEncryptPrivateCredentials(
                         *pExportPassphrase, bImporting))
            return false;
    }

    return true;
}

std::uint64_t Nym::Revision() const { return revision_.load(); }

void Nym::revoke_contact_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    std::list<std::string> revokedIDs;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->RevokeContactCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { m_listRevokedIDs.push_back(it); }
}

void Nym::revoke_verification_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    std::list<std::string> revokedIDs;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->RevokeVerificationCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { m_listRevokedIDs.push_back(it); }
}

std::shared_ptr<const proto::Credential> Nym::RevokedCredentialContents(
    const std::string& id) const
{
    eLock lock(shared_lock_);

    std::shared_ptr<const proto::Credential> output;

    auto iter = m_mapRevokedSets.find(id);

    if (m_mapRevokedSets.end() != iter) {
        output = iter->second->GetMasterCredential().Serialized(
            AS_PUBLIC, WITH_SIGNATURES);
    }

    return output;
}

void Nym::SaveCredentialsToTag(
    Tag& parent,
    String::Map* pmapPubInfo,
    String::Map* pmapPriInfo) const
{
    // IDs for revoked child credentials are saved here.
    for (auto& it : m_listRevokedIDs) {
        std::string str_revoked_id = it;
        TagPtr pTag(new Tag("revokedCredential"));
        pTag->add_attribute("ID", str_revoked_id);
        parent.add_tag(pTag);
    }

    // Serialize master and sub-credentials here.
    for (auto& it : m_mapCredentialSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(
            parent,
            m_listRevokedIDs,
            pmapPubInfo,
            pmapPriInfo,
            true);  // bShowRevoked=false by default (true here), bValid=true
    }

    // Serialize Revoked master credentials here, including their child key
    // credentials.
    for (auto& it : m_mapRevokedSets) {
        CredentialSet* pCredential = it.second;
        OT_ASSERT(nullptr != pCredential);

        pCredential->SerializeIDs(
            parent,
            m_listRevokedIDs,
            pmapPubInfo,
            pmapPriInfo,
            true,
            false);  // bShowRevoked=false by default. (Here it's true.)
                     // bValid=true by default. Here is for revoked, so false.
    }
}

bool Nym::SavePseudonymWallet(Tag& parent) const
{
    sLock lock(shared_lock_);

    auto nymID = String::Factory(m_nymID);

    // Name is in the clear in memory,
    // and base64 in storage.
    auto ascName = Armored::Factory();
    if (!alias_.empty()) {
        auto temp = String::Factory(alias_);
        ascName->SetString(temp, false);  // linebreaks == false
    }

    TagPtr pTag(new Tag("pseudonym"));

    pTag->add_attribute("name", !alias_.empty() ? ascName->Get() : "");
    pTag->add_attribute("nymID", nymID->Get());

    parent.add_tag(pTag);

    return true;
}

serializedCredentialIndex Nym::SerializeCredentialIndex(
    const CredentialIndexModeFlag mode) const
{
    sLock lock(shared_lock_);

    serializedCredentialIndex index;

    index.set_version(version_);
    auto nymID = String::Factory(m_nymID);
    index.set_nymid(nymID->Get());

    if (CREDENTIAL_INDEX_MODE_ONLY_IDS == mode) {
        index.set_mode(mode_);

        if (proto::CREDINDEX_PRIVATE == mode_) { index.set_index(index_); }
    } else {
        index.set_mode(proto::CREDINDEX_PUBLIC);
    }

    index.set_revision(revision_.load());
    *(index.mutable_source()) = *(source_->Serialize());

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            SerializedCredentialSet credset = it.second->Serialize(mode);
            auto pCredSet = index.add_activecredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    for (auto& it : m_mapRevokedSets) {
        if (nullptr != it.second) {
            SerializedCredentialSet credset = it.second->Serialize(mode);
            auto pCredSet = index.add_revokedcredentials();
            *pCredSet = *credset;
            pCredSet = nullptr;
        }
    }

    return index;
}

void Nym::SerializeNymIDSource(Tag& parent) const
{
    // We encode these before storing.
    if (source_) {

        TagPtr pTag(new Tag("nymIDSource", source_->asString()->Get()));

        if (m_strDescription->Exists()) {
            auto ascDescription = Armored::Factory();
            ascDescription->SetString(
                m_strDescription,
                false);  // bLineBreaks=true by default.

            pTag->add_attribute("Description", ascDescription->Get());
        }
        parent.add_tag(pTag);
    }
}

bool Nym::set_contact_data(const eLock& lock, const proto::ContactData& data)
{
    OT_ASSERT(verify_lock(lock));

    auto version = proto::NymRequiredVersion(data.version(), version_);

    if (!version || version > NYM_UPGRADE_VERSION) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Contact data version not supported by this nym.")
            .Flush();
        return false;
    }

    if (false == has_capability(lock, NymCapability::SIGN_CHILDCRED)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": This nym can not be modified.")
            .Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE, false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid contact data.").Flush();

        return false;
    }

    revoke_contact_credentials(lock);

    if (add_contact_credential(lock, data)) {

        return update_nym(lock, version);
    }

    return false;
}

void Nym::SetAlias(const std::string& alias)
{
    eLock lock(shared_lock_);

    alias_ = alias;
    revision_++;
}

bool Nym::SetCommonName(const std::string& name)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    contact_data_.reset(new ContactData(contact_data_->SetCommonName(name)));

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::SetContactData(const proto::ContactData& data)
{
    eLock lock(shared_lock_);

    contact_data_.reset(
        new ContactData(m_nymID->str(), NYM_CONTACT_DATA_VERSION, data));

    return set_contact_data(lock, data);
}

bool Nym::SetScope(
    const proto::ContactItemType type,
    const std::string& name,
    const bool primary)
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    if (proto::CITEMTYPE_UNKNOWN != contact_data_->Type()) {
        contact_data_.reset(
            new ContactData(contact_data_->SetName(name, primary)));
    } else {
        contact_data_.reset(
            new ContactData(contact_data_->SetScope(type, name)));
    }

    OT_ASSERT(contact_data_);

    return set_contact_data(lock, contact_data_->Serialize());
}

bool Nym::SetVerificationSet(const proto::VerificationSet& data)
{
    eLock lock(shared_lock_);

    if (false == has_capability(lock, NymCapability::SIGN_CHILDCRED)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": This nym can not be modified.")
            .Flush();

        return false;
    }

    revoke_verification_credentials(lock);

    if (add_verification_credential(lock, data)) {

        return update_nym(lock, version_);
    }

    return false;
}

std::string Nym::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->SocialMediaProfiles(type, active);
}

const std::set<proto::ContactItemType> Nym::SocialMediaProfileTypes() const
{
    sLock lock(shared_lock_);

    return contact_data_->SocialMediaProfileTypes();
}

std::unique_ptr<OTPassword> Nym::TransportKey(Data& pubkey) const
{
    bool found{false};
    auto privateKey = std::make_unique<OTPassword>();

    OT_ASSERT(privateKey);

    sLock lock(shared_lock_);

    for (auto& it : m_mapCredentialSets) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const CredentialSet& credSet = *it.second;
            found = credSet.TransportKey(pubkey, *privateKey);

            if (found) { break; }
        }
    }

    OT_ASSERT(found);

    return privateKey;
}

bool Nym::update_nym(const eLock& lock, const std::int32_t version)
{
    OT_ASSERT(verify_lock(lock));

    if (verify_pseudonym(lock)) {
        // Upgrade version
        if (version > version_) { version_ = version; }

        ++revision_;

        return true;
    }

    return false;
}

std::unique_ptr<proto::VerificationSet> Nym::VerificationSet() const
{
    std::unique_ptr<proto::VerificationSet> verificationSet;

    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            it.second->GetVerificationSet(verificationSet);
        }
    }

    return verificationSet;
}

bool Nym::Verify(const Data& plaintext, const proto::Signature& sig) const
{
    for (auto& it : m_mapCredentialSets) {
        if (nullptr != it.second) {
            if (it.second->Verify(plaintext, sig)) { return true; }
        }
    }

    return false;
}

bool Nym::VerifyPseudonym() const
{
    eLock lock(shared_lock_);

    return verify_pseudonym(lock);
}

bool Nym::verify_pseudonym(const eLock& lock) const
{
    // If there are credentials, then we verify the Nym via his credentials.
    if (!m_mapCredentialSets.empty()) {
        // Verify Nym by his own credentials.
        for (const auto& it : m_mapCredentialSets) {
            const CredentialSet* pCredential = it.second;
            OT_ASSERT(nullptr != pCredential);

            const OTIdentifier theCredentialNymID =
                Identifier::Factory(pCredential->GetNymID());
            if (m_nymID != theCredentialNymID) {
                LogNormal(OT_METHOD)(__FUNCTION__)(": Credential NymID (")(
                    pCredential->GetNymID())(") doesn't match actual NymID: ")(
                    m_nymID->str())(".")
                    .Flush();
                return false;
            }

            // Verify all Credentials in the CredentialSet, including source
            // verification for the master credential.
            if (!pCredential->VerifyInternally()) {
                LogNormal(OT_METHOD)(__FUNCTION__)(": Credential (")(
                    pCredential->GetMasterCredID())(
                    ") failed its own internal verification.")
                    .Flush();
                return false;
            }
        }
        return true;
    }
    LogOutput(OT_METHOD)(__FUNCTION__)(": No credentials.").Flush();
    return false;
}

bool Nym::WriteCredentials() const
{
    sLock lock(shared_lock_);

    for (auto& it : m_mapCredentialSets) {
        if (!it.second->WriteCredentials()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save credentials.")
                .Flush();

            return false;
        }
    }

    return true;
}

Nym::~Nym() { ClearCredentials(); }
}  // namespace opentxs
