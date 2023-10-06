// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/Contract.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <irrxml/irrXML.hpp>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <memory>
#include <utility>

#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/crypto/OTSignatureMetadata.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/common/OTStorage.hpp"

namespace opentxs
{
Contract::Contract(const api::Session& api)
    : Contract(
          api,
          String::Factory(),
          String::Factory(),
          String::Factory(),
          String::Factory())
{
}

Contract::Contract(
    const api::Session& api,
    const String& name,
    const String& foldername,
    const String& filename,
    const String& strID)
    : api_{api}
    , name_(name)
    , foldername_(foldername)
    , filename_(filename)
    , id_(api_.Factory().IdentifierFromBase58(strID.Bytes()))
    , xml_unsigned_(StringXML::Factory())
    , raw_file_(String::Factory())
    , sig_hash_type_(crypto::HashType::Error)
    , contract_type_(String::Factory("CONTRACT"))
    , nyms_()
    , list_signatures_()
    , version_(String::Factory("2.0"))
    , entity_short_name_(String::Factory())
    , entity_long_name_(String::Factory())
    , entity_email_(String::Factory())
    , conditions_()
{
}

Contract::Contract(const api::Session& api, const String& strID)
    : Contract(
          api,
          String::Factory(),
          String::Factory(),
          String::Factory(),
          strID)
{
}

Contract::Contract(const api::Session& api, const identifier::Generic& theID)
    : Contract(api, String::Factory(theID, api.Crypto()))
{
}

void Contract::SetIdentifier(const identifier::Generic& theID) { id_ = theID; }

// The name, filename, version, and ID loaded by the wallet
// are NOT released here, since they are used immediately after
// the Release() call in LoadContract(). Really I just want to
// "Release" the stuff that is about to be loaded, not the stuff
// that I need to load it!
void Contract::Release_Contract()
{
    sig_hash_type_ = crypto::HashType::Error;
    xml_unsigned_->Release();
    raw_file_->Release();

    ReleaseSignatures();

    conditions_.clear();

    nyms_.clear();
}

void Contract::Release()
{
    Release_Contract();

    // No call to ot_super::Release() here, since Contract
    // is the base class.
}

Contract::~Contract() { Release_Contract(); }

auto Contract::SaveToContractFolder() -> bool
{
    OTString strFoldername(
        String::Factory(api_.Internal().Legacy().Contract())),
        strFilename = String::Factory();

    GetIdentifier(strFilename);

    // These are already set in SaveContract(), called below.
    //    foldername_    = strFoldername;
    //    filename_    = strFilename;

    LogVerbose()()("Saving asset contract to ")("disk... ").Flush();

    return SaveContract(strFoldername->Get(), strFilename->Get());
}

void Contract::GetFilename(String& strFilename) const
{
    String::Factory(strFilename.Get()) = filename_;
}

void Contract::GetIdentifier(identifier::Generic& theIdentifier) const
{
    theIdentifier = id_;
}

void Contract::GetIdentifier(String& theIdentifier) const
{
    id_.GetString(api_.Crypto(), theIdentifier);
}

// Make sure this contract checks out. Very high level.
// Verifies ID, existence of public key, and signature.
//
auto Contract::VerifyContract() const -> bool
{
    // Make sure that the supposed Contract ID that was set is actually
    // a hash of the contract file, signatures and all.
    if (!VerifyContractID()) {
        LogDetail()()("Failed verifying contract ID.").Flush();
        return false;
    }

    // Make sure we are able to read the official "contract" public key out of
    // this contract.
    auto pNym = GetContractPublicNym();

    if (nullptr == pNym) {
        LogConsole()()("Failed retrieving public nym from contract.").Flush();
        return false;
    }

    if (!VerifySignature(*pNym)) {
        const auto& nymID = pNym->ID();
        const auto strNymID = String::Factory(nymID, api_.Crypto());
        LogConsole()()(
            "Failed verifying the contract's signature against the public key "
            "that was retrieved from the contract, with key ID: ")(
            strNymID.get())(".")
            .Flush();
        return false;
    }

    LogDetail()()(
        "Verified -- The Contract ID from the wallet matches the "
        "newly-calculated hash of the contract file. "
        "Verified -- A standard \"contract\" Public Key or x509 Cert WAS "
        "found inside the contract. "
        "Verified -- And the **SIGNATURE VERIFIED** with THAT key.")
        .Flush();
    return true;
}

void Contract::CalculateContractID(identifier::Generic& newID) const
{
    // may be redundant...
    UnallocatedCString str_Trim(raw_file_->Get());
    UnallocatedCString str_Trim2 = String::trim(str_Trim);
    auto strTemp = String::Factory(str_Trim2.c_str());
    newID = api_.Factory().IdentifierFromPreimage(strTemp->Bytes());

    if (newID.empty()) {
        LogError()()("Error calculating Contract digest.").Flush();
    }
}

void Contract::CalculateAndSetContractID(identifier::Generic& newID)
{
    CalculateContractID(newID);
    SetIdentifier(newID);
}

auto Contract::VerifyContractID() const -> bool
{
    auto newID = identifier::Generic{};
    CalculateContractID(newID);

    // newID now contains the Hash aka Message Digest aka Fingerprint
    // aka thumbprint aka "IDENTIFIER" of the Contract.
    //
    // Now let's compare that identifier to the one already loaded by the wallet
    // for this contract and make sure they MATCH.

    // I use the == operator here because there is no != operator at this time.
    // That's why you see the ! outside the parenthesis.
    //
    if (!(id_ == newID)) {
        auto str1 = String::Factory(id_, api_.Crypto()),
             str2 = String::Factory(newID, api_.Crypto());

        LogConsole()()(
            "Hashes do NOT match in Contract::VerifyContractID. "
            "Expected: ")(str1.get())(". ")("Actual: ")(str2.get())(".")
            .Flush();
        return false;
    } else {
        auto str1 = String::Factory();
        newID.GetString(api_.Crypto(), str1);
        LogDetail()()("Contract ID *SUCCESSFUL* match to "
                      "hash of contract file: ")(str1.get())
            .Flush();
        return true;
    }
}

auto Contract::GetContractPublicNym() const -> Nym_p
{
    for (const auto& it : nyms_) {
        Nym_p pNym = it.second;
        assert_false(nullptr == pNym, "nullptr pseudonym pointer");

        // We favor the new "credential" system over the old "public key"
        // system.
        // No one will ever actually put BOTH in a single contract. But if they
        // do,
        // we favor the new version over the old.
        if (it.first == "signer") {
            return pNym;
        }
        // TODO have a place for hardcoded values like this.
        else if (it.first == "contract") {
            // We're saying here that every contract has to have a key tag
            // called "contract"
            // where the official public key can be found for it and for any
            // contract.
            return pNym;
        }
    }

    return nullptr;
}

// This is the one that you will most likely want to call.
// It actually attaches the resulting signature to this contract.
// If you want the signature to remain on the contract and be handled
// internally, then this is what you should call.
//
auto Contract::SignContract(
    const identity::Nym& nym,
    const PasswordPrompt& reason) -> bool
{
    auto sig = Signature::Factory(api_);
    bool bSigned = SignContract(nym, sig, reason);

    if (bSigned) {
        list_signatures_.emplace_back(std::move(sig));
    } else {
        LogError()()("Failure while calling "
                     "SignContract(nym, sig, reason).")
            .Flush();
    }

    return bSigned;
}

// Signs using authentication key instead of signing key.
//
auto Contract::SignContractAuthent(
    const identity::Nym& nym,
    const PasswordPrompt& reason) -> bool
{
    auto sig = Signature::Factory(api_);
    bool bSigned = SignContractAuthent(nym, sig, reason);

    if (bSigned) {
        list_signatures_.emplace_back(std::move(sig));
    } else {
        LogError()()("Failure while calling "
                     "SignContractAuthent(nym, sig, "
                     "reason).")
            .Flush();
    }

    return bSigned;
}

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
auto Contract::SignContract(
    const identity::Nym& nym,
    Signature& theSignature,
    const PasswordPrompt& reason) -> bool
{
    const auto& key = nym.GetPrivateSignKey();
    sig_hash_type_ = key.PreferredHash();

    return SignContract(key, theSignature, sig_hash_type_, reason);
}

// Uses authentication key instead of signing key.
auto Contract::SignContractAuthent(
    const identity::Nym& nym,
    Signature& theSignature,
    const PasswordPrompt& reason) -> bool
{
    const auto& key = nym.GetPrivateAuthKey();
    sig_hash_type_ = key.PreferredHash();

    return SignContract(key, theSignature, sig_hash_type_, reason);
}

// Normally you'd use Contract::SignContract(const identity::Nym& nym)...
// Normally you WOULDN'T use this function SignWithKey.
// But this is here anyway for those peculiar places where you need it. For
// example,
// when first creating a Nym, you generate the master credential as part of
// creating
// the Nym, and the master credential has to sign itself, and it therefore needs
// to be
// able to "sign a contract" at a high level using purely the key, without
// having the Nym
// ready yet to signing anything with.
//
auto Contract::SignWithKey(
    const crypto::asymmetric::Key& key,
    const PasswordPrompt& reason) -> bool
{
    auto sig = Signature::Factory(api_);
    sig_hash_type_ = key.PreferredHash();
    bool bSigned = SignContract(key, sig, sig_hash_type_, reason);

    if (bSigned) {
        list_signatures_.emplace_back(std::move(sig));
    } else {
        LogError()()("Failure while calling SignContract(nym, sig).").Flush();
    }

    return bSigned;
}

// Done: When signing a contract, need to record the metadata into the signature
// object here.

// We will know if the key is signing, authentication, or encryption key
// because?
// Because we used the Nym to choose it! In which case we should have a default
// option,
// and also some other function with a new name that calls SignContract and
// CHANGES that default
// option.
// For example, SignContract(bool bUseAuthenticationKey=false)
// Then: SignContractAuthentication() { return SignContract(true); }
//
// In most cases we actually WILL want the signing key, since we are actually
// signing contracts
// such as cash withdrawals, etc. But when the Nym stores something for himself
// locally, or when
// sending messages, those will use the authentication key.
//
// We'll also have the ability to SWITCH the key which is important because it
// raises the
// question, how do we CHOOSE the key? On my phone I might use a different key
// than on my iPad.
// nym should either know already (GetPrivateKey being intelligent) or it
// must be passed in
// (Into the below versions of SignContract.)
//
// If key knows its type (A|E|S) the next question is, does it know its other
// metadata?
// It certainly CAN know, can't it? Especially if it's being loaded from
// credentials in the
// first place. And if not, well then the data's not there and it's not added to
// the signature.
// (Simple.) So I will put the Signature Metadata into its own class, so not
// only a signature
// can use it, but also the crypto::asymmetric::Key class can use it and also
// Credential can use it.
// Then Contract just uses it if it's there. Also we don't have to pass it in
// here as separate
// parameters. At most we have to figure out which private key to get above, in
// nym.GetPrivateKey()
// Worst case maybe put a loop, and see which of the private keys inside that
// Nym, in its credentials,
// is actually loaded and available. Then just have GetPrivateKey return THAT
// one. Similarly, later
// on, in VerifySignature, we'll pass the signature itself into the Nym so that
// the Nym can use it
// to help search for the proper public key to use for verifying, based on that
// metadata.
//
// This is all great because it means the only real change I need to do here now
// is to see if
// key.HasMetadata and if so, just copy it directly over to theSignature's
// Metadata.
//

// The output signature will be in theSignature.
// It is NOT attached to the contract.  This is just a utility function.
//
auto Contract::SignContract(
    const crypto::asymmetric::Key& key,
    Signature& theSignature,
    const crypto::HashType hashType,
    const PasswordPrompt& reason) -> bool
{
    // We assume if there's any important metadata, it will already
    // be on the key, so we just copy it over to the signature.
    const auto* metadata = key.Internal().GetMetadata();

    if (nullptr != metadata) { theSignature.getMetaData() = *(metadata); }

    // Update the contents, (not always necessary, many contracts are read-only)
    // This is where we provide an overridable function for the child classes
    // that
    // need to update their contents at this point.
    // But the Contract version of this function is actually empty, since the
    // default behavior is that contract contents don't change.
    // (Accounts and Messages being two big exceptions.)
    //
    UpdateContents(reason);

    const auto& engine = key.Internal().Provider();

    if (false == engine.SignContract(
                     api_,
                     trim(xml_unsigned_),
                     key.PrivateKey(reason),
                     hashType,
                     theSignature)) {
        LogError()()("engine.SignContract returned false.").Flush();
        return false;
    }

    return true;
}

auto Contract::VerifySigAuthent(const identity::Nym& nym) const -> bool
{
    auto strNymID = String::Factory();
    nym.GetIdentifier(strNymID);
    char cNymID = '0';
    std::uint32_t uIndex = 3;
    const bool bNymID = strNymID->At(uIndex, cNymID);

    for (const auto& sig : list_signatures_) {
        if (bNymID && sig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the fourth character
            // of the NymID that signed it. We know the fourth character of the
            // NymID who's trying to verify it. Thus, if they don't match, we
            // can skip this signature without having to try to verify it at
            // all.
            if (sig->getMetaData().FirstCharNymID() != cNymID) { continue; }
        }

        if (VerifySigAuthent(nym, sig)) { return true; }
    }

    return false;
}

auto Contract::VerifySignature(const identity::Nym& nym) const -> bool
{
    auto strNymID = String::Factory(nym.ID(), api_.Crypto());
    char cNymID = '0';
    std::uint32_t uIndex = 3;
    const bool bNymID = strNymID->At(uIndex, cNymID);

    for (const auto& sig : list_signatures_) {
        if (bNymID && sig->getMetaData().HasMetadata()) {
            // If the signature has metadata, then it knows the fourth character
            // of the NymID that signed it. We know the fourth character of the
            // NymID who's trying to verify it. Thus, if they don't match, we
            // can skip this signature without having to try to verify it at
            // all.
            if (sig->getMetaData().FirstCharNymID() != cNymID) { continue; }
        }

        if (VerifySignature(nym, sig)) { return true; }
    }

    return false;
}

auto Contract::VerifyWithKey(const crypto::asymmetric::Key& key) const -> bool
{
    for (const auto& sig : list_signatures_) {
        const auto* metadata = key.Internal().GetMetadata();

        if ((nullptr != metadata) && metadata->HasMetadata() &&
            sig->getMetaData().HasMetadata()) {
            // Since key and signature both have metadata, we can use it
            // to skip signatures which don't match this key.
            //
            if (sig->getMetaData() != *(metadata)) { continue; }
        }

        if (VerifySignature(key, sig, sig_hash_type_)) { return true; }
    }

    return false;
}

// Like VerifySignature, except it uses the authentication key instead of the
// signing key. (Like for sent messages or stored files, where you want a
// signature but you don't want a legally binding signature, just a technically
// secure signature.)
auto Contract::VerifySigAuthent(
    const identity::Nym& nym,
    const Signature& theSignature) const -> bool
{
    crypto::key::Keypair::Keys listOutput;

    const std::int32_t nCount = nym.Internal().GetPublicKeysBySignature(
        listOutput, theSignature, 'A');  // 'A' for authentication key.

    if (nCount > 0)  // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            const auto* pKey = it;
            assert_false(nullptr == pKey);

            if (VerifySignature(*pKey, theSignature, sig_hash_type_)) {
                return true;
            }
        }
    } else {
        auto strNymID = String::Factory();
        nym.GetIdentifier(strNymID);
        LogDetail()()("Tried to grab a list of keys from this Nym "
                      "(")(strNymID.get())(
            ") which might match this signature, but recovered none. "
            "Therefore, will attempt to verify using the Nym's default public "
            "AUTHENTICATION key.")
            .Flush();
    }
    // else found no keys.

    return VerifySignature(
        nym.GetPublicAuthKey(), theSignature, sig_hash_type_);
}

// The only different between calling this with a Nym and calling it with an
// Asymmetric Key is that
// the key gives you the choice of hash algorithm, whereas the nym version uses
// hash_type_ to decide
// for you.  Choose the function you prefer, you can do it either way.
//
auto Contract::VerifySignature(
    const identity::Nym& nym,
    const Signature& theSignature) const -> bool
{
    crypto::key::Keypair::Keys listOutput;

    const std::int32_t nCount = nym.Internal().GetPublicKeysBySignature(
        listOutput, theSignature, 'S');  // 'S' for signing key.

    if (nCount > 0)  // Found some (potentially) matching keys...
    {
        for (auto& it : listOutput) {
            const auto* pKey = it;
            assert_false(nullptr == pKey);

            if (VerifySignature(*pKey, theSignature, sig_hash_type_)) {
                return true;
            }
        }
    } else {
        auto strNymID = String::Factory();
        nym.GetIdentifier(strNymID);
        LogDetail()()("Tried to grab a list of keys from this Nym "
                      "(")(strNymID.get())(
            ") which might match this signature, but recovered none. "
            "Therefore, will attempt to verify using the Nym's default public "
            "SIGNING key.")
            .Flush();
    }
    // else found no keys.

    return VerifySignature(
        nym.GetPublicSignKey(), theSignature, sig_hash_type_);
}

auto Contract::VerifySignature(
    const crypto::asymmetric::Key& key,
    const Signature& theSignature,
    const crypto::HashType hashType) const -> bool
{
    const auto* metadata = key.Internal().GetMetadata();

    // See if this key could possibly have even signed this signature.
    // (The metadata may eliminate it as a possibility.)
    if ((nullptr != metadata) && metadata->HasMetadata() &&
        theSignature.getMetaData().HasMetadata()) {
        if (theSignature.getMetaData() != *(metadata)) { return false; }
    }

    const auto& engine = key.Internal().Provider();

    if (false == engine.VerifyContractSignature(
                     api_,
                     trim(xml_unsigned_),
                     key.PublicKey(),
                     theSignature,
                     hashType)) {
        LogTrace()()("engine.VerifyContractSignature returned false.").Flush();

        return false;
    }

    return true;
}

void Contract::ReleaseSignatures() { list_signatures_.clear(); }

auto Contract::DisplayStatistics(String& strContents) const -> bool
{
    // Subclasses may override this.
    static auto msg = String::Factory(
        UnallocatedCString{"ERROR:  Contract::DisplayStatistics was called "
                           "instead of a subclass...\n"});
    strContents.Concatenate(msg);

    return false;
}

auto Contract::SaveContractWallet(Tag&) const -> bool
{
    // Subclasses may use this.

    return false;
}

auto Contract::SaveContents(std::ofstream& ofs) const -> bool
{
    ofs << xml_unsigned_;

    return true;
}

// Saves the unsigned XML contents to a string
auto Contract::SaveContents(String& strContents) const -> bool
{
    strContents.Concatenate(xml_unsigned_);

    return true;
}

// Save the contract member variables into the raw_file_ variable
auto Contract::SaveContract() -> bool
{
    auto strTemp = String::Factory();
    bool bSuccess = RewriteContract(strTemp);

    if (bSuccess) {
        raw_file_->Set(strTemp);

        // RewriteContract() already does this.
        //
        //        UnallocatedCString str_Trim(strTemp.Get());
        //        UnallocatedCString str_Trim2 = OTString::trim(str_Trim);
        //        raw_file_.Set(str_Trim2.c_str());
    }

    return bSuccess;
}

void Contract::UpdateContents(const PasswordPrompt& reason)
{
    // Deliberately left blank.
    //
    // Some child classes may need to perform work here
    // (OTAccount and OTMessage, for example.)
    //
    // This function is called just prior to the signing of a contract.

    // Update: MOST child classes actually use this.
    // The server and asset contracts are not meant to ever change after
    // they are signed. However, many other contracts are meant to change
    // and be re-signed. (You cannot change something without signing it.)
    // (So most child classes override this method.)
}

// Saves the raw (pre-existing) contract text to any string you want to pass in.
auto Contract::SaveContractRaw(String& strOutput) const -> bool
{
    strOutput.Concatenate(raw_file_);

    return true;
}

// Takes the pre-existing XML contents (WITHOUT signatures) and re-writes
// into strOutput the appearance of raw_data_, adding the pre-existing
// signatures along with new signature bookends.. (The caller actually passes
// raw_data_ into this function...)
//
auto Contract::RewriteContract(String& strOutput) const -> bool
{
    auto strContents = String::Factory();
    SaveContents(strContents);

    return AddBookendsAroundContent(
        strOutput,
        strContents,
        contract_type_,
        sig_hash_type_,
        list_signatures_);
}

auto Contract::SaveContract(const char* szFoldername, const char* szFilename)
    -> bool
{
    assert_false(nullptr == szFilename, "Null filename");
    assert_false(nullptr == szFoldername, "Null foldername");

    foldername_->Set(szFoldername);
    filename_->Set(szFilename);

    return WriteContract(szFoldername, szFilename);
}

auto Contract::WriteContract(
    const UnallocatedCString& folder,
    const UnallocatedCString& filename) const -> bool
{
    assert_true(folder.size() > 2);
    assert_true(filename.size() > 2);

    if (!raw_file_->Exists()) {
        LogError()()("Error saving file (contract contents are "
                     "empty): ")(folder)('/')(filename)
            .Flush();

        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(api_.Crypto(), raw_file_);

    if (false == ascTemp->WriteArmoredString(strFinal, contract_type_->Get())) {
        LogError()()("Error saving file (failed writing armored "
                     "string): ")(folder)('/')(filename)
            .Flush();

        return false;
    }

    const bool bSaved = OTDB::StorePlainString(
        api_,
        strFinal->Get(),
        api_.DataFolder().string(),
        folder,
        filename,
        "",
        "");

    if (!bSaved) {
        LogError()()("Error saving file: ")(folder)('/')(filename).Flush();

        return false;
    }

    return true;
}

// assumes filename_ is already set.
// Then it reads that file into a string.
// Then it parses that string into the object.
auto Contract::LoadContract() -> bool
{
    Release();
    LoadContractRawFile();  // opens filename_ and reads into raw_file_

    return ParseRawFile();  // Parses raw_file_ into the various member
                            // variables.
}

// The entire Raw File, signatures and all, is used to calculate the hash
// value that becomes the ID of the contract. If you change even one letter,
// then you get a different ID.
// This applies to all contracts except accounts, since their contents must
// change periodically, their ID is not calculated from a hash of the file,
// but instead is chosen at random when the account is created.
auto Contract::LoadContractRawFile() -> bool
{
    const char* szFoldername = foldername_->Get();
    const char* szFilename = filename_->Get();

    if (!foldername_->Exists() || !filename_->Exists()) { return false; }

    if (!OTDB::Exists(
            api_,
            api_.DataFolder().string(),
            szFoldername,
            szFilename,
            "",
            "")) {
        LogVerbose()()("File does not "
                       "exist: ")(szFoldername)('/')(szFilename)
            .Flush();
        return false;
    }

    auto strFileContents = String::Factory(OTDB::QueryPlainString(
        api_,
        api_.DataFolder().string(),
        szFoldername,
        szFilename,
        "",
        ""));  // <===
               // LOADING
               // FROM
               // DATA
               // STORE.

    if (!strFileContents->Exists()) {
        LogError()()("Error reading "
                     "file: ")(szFoldername)('/')(szFilename)
            .Flush();
        return false;
    }

    if (false == strFileContents->DecodeIfArmored(
                     api_.Crypto()))  // bEscapedIsAllowed=true
                                      // by default.
    {
        LogError()()(
            "Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strFileContents.get())(".")
            .Flush();
        return false;
    }

    // At this point, strFileContents contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    raw_file_->Set(strFileContents);

    return raw_file_->Exists();
}

auto Contract::LoadContract(const char* szFoldername, const char* szFilename)
    -> bool
{
    Release();

    foldername_->Set(szFoldername);
    filename_->Set(szFilename);

    // opens filename_ and reads into raw_file_
    if (LoadContractRawFile()) {
        return ParseRawFile();  // Parses raw_file_ into the various
                                // member variables.
    } else {
        LogDetail()()("Failed loading raw contract file: ")(foldername_.get())(
            " file")(filename_.get())
            .Flush();
    }
    return false;
}

// Just like it says. If you have a contract in string form, pass it in
// here to import it.
auto Contract::LoadContractFromString(const String& theStr) -> bool
{
    Release();

    if (!theStr.Exists()) {
        LogError()()("ERROR: Empty string passed in...").Flush();
        return false;
    }

    auto strContract = String::Factory(theStr.Get());

    if (false ==
        strContract->DecodeIfArmored(api_.Crypto()))  // bEscapedIsAllowed=true
                                                      // by default.
    {
        LogError()()(
            "ERROR: Input string apparently was encoded and then failed "
            "decoding. Contents: ")(theStr)(".")
            .Flush();
        return false;
    }

    raw_file_->Set(strContract);

    // This populates xml_unsigned_ with the contents of raw_file_ (minus
    // bookends, signatures, etc. JUST the XML.)
    bool bSuccess = ParseRawFile();  // It also parses into the various
                                     // member variables.

    // Removed:
    // This was the bug where the version changed from 75 to 75c, and suddenly
    // contract ID was wrong...
    //
    // If it was a success, save back to raw_file_ again so
    // the format is consistent and hashes will calculate properly.
    //    if (bSuccess)
    //    {
    //        // Basically we take the xml_unsigned_ that we parsed out of the
    // raw file before,
    //        // then we use that to generate the raw file again, re-attaching
    // the signatures.
    //        // This function does that.
    //        SaveContract();
    //    }

    return bSuccess;
}

auto Contract::ParseRawFile() -> bool
{
    auto buffer1 = std::array<char, 2100>{};  // a bit bigger than 2048, just
                                              // for safety reasons.
    Signature* pSig{nullptr};
    UnallocatedCString line;
    bool bSignatureMode = false;           // "currently in signature mode"
    bool bContentMode = false;             // "currently in content mode"
    bool bHaveEnteredContentMode = false;  // "have yet to enter content mode"

    if (!raw_file_->GetLength()) {
        LogError()()("Empty raw_file_ in Contract::ParseRawFile. "
                     "Filename: ")(foldername_.get())('/')(filename_.get())
            .Flush();
        return false;
    }

    // This is redundant (I thought) but the problem hasn't cleared up yet.. so
    // trying to really nail it now.
    UnallocatedCString str_Trim(raw_file_->Get());
    UnallocatedCString str_Trim2 = String::trim(str_Trim);
    raw_file_->Set(str_Trim2.c_str());

    bool bIsEOF = false;
    raw_file_->reset();

    do {
        // the call returns true if there's more to read, and false if there
        // isn't.
        bIsEOF = !(raw_file_->sgets(buffer1.data(), 2048));
        line = buffer1.data();

        if (line.length() < 2) {
            if (bSignatureMode) { continue; }
        }

        // if we're on a dashed line...
        else if (line.at(0) == '-') {
            if (bSignatureMode) {
                // we just reached the end of a signature
                bSignatureMode = false;
                continue;
            }

            // if I'm NOT in signature mode, and I just hit a dash, that means
            // there
            // are only four options:

            // a. I have not yet even entered content mode, and just now
            // entering it for the first time.
            if (!bHaveEnteredContentMode) {
                if ((line.length() > 3) &&
                    (line.find("BEGIN") != UnallocatedCString::npos) &&
                    line.at(1) == '-' && line.at(2) == '-' &&
                    line.at(3) == '-') {
                    bHaveEnteredContentMode = true;
                    bContentMode = true;
                    continue;
                } else {
                    continue;
                }

            }

            // b. I am now entering signature mode!
            else if (
                line.length() > 3 &&
                line.find("SIGNATURE") != UnallocatedCString::npos &&
                line.at(1) == '-' && line.at(2) == '-' && line.at(3) == '-') {
                bSignatureMode = true;
                bContentMode = false;
                list_signatures_.emplace_back(Signature::Factory(api_));
                pSig = &(list_signatures_.rbegin()->get());

                continue;
            }
            // c. There is an error in the file!
            else if (
                line.length() < 3 || line.at(1) != ' ' || line.at(2) != '-') {
                LogConsole()()("Error in contract ")(filename_.get())(
                    ": A dash at the beginning of the line should be followed "
                    "by a space and another dash: ")(raw_file_.get())(".")
                    .Flush();
                return false;
            }
            // d. It is an escaped dash, and therefore kosher, so I merely
            // remove the escape and add it.
            // I've decided not to remove the dashes but to keep them as part of
            // the signed content.
            // It's just much easier to deal with that way. The input code will
            // insert the extra dashes.
            // line += 2;
        }

        // Else we're on a normal line, not a dashed line.
        else {
            if (bHaveEnteredContentMode) {
                if (bSignatureMode) {
                    if (line.length() < 2) {
                        LogDebug()()("Skipping short line...").Flush();

                        if (bIsEOF || !raw_file_->sgets(buffer1.data(), 2048)) {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected EOF after short line.")
                                .Flush();
                            return false;
                        }

                        continue;
                    } else if (line.compare(0, 8, "Version:") == 0) {
                        LogDebug()()("Skipping version section...").Flush();

                        if (bIsEOF || !raw_file_->sgets(buffer1.data(), 2048)) {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected EOF after Version: .")
                                .Flush();
                            return false;
                        }

                        continue;
                    } else if (line.compare(0, 8, "Comment:") == 0) {
                        LogDebug()()("Skipping comment section..").Flush();

                        if (bIsEOF || !raw_file_->sgets(buffer1.data(), 2048)) {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected EOF after Comment: .")
                                .Flush();
                            return false;
                        }

                        continue;
                    }
                    if (line.compare(0, 5, "Meta:") == 0) {
                        LogDebug()()("Collecting signature metadata...")
                            .Flush();

                        if (line.length() != 13)  // "Meta:    knms" (It will
                                                  // always be exactly 13
                        // characters std::int64_t.) knms represents the
                        // first characters of the Key type, NymID,
                        // Master Cred ID, and ChildCred ID. Key type is
                        // (A|E|S) and the others are base64.
                        {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected length for Meta: comment.")
                                .Flush();
                            return false;
                        }

                        if (nullptr == pSig) {
                            LogConsole()()("Corrupted signature").Flush();

                            return false;
                        }

                        auto& sig = *pSig;

                        if (false == sig.getMetaData().SetMetadata(
                                         line.at(9),
                                         line.at(10),
                                         line.at(11),
                                         line.at(12)))  // "knms" from "Meta:
                                                        // knms"
                        {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected metadata in the Meta: comment. "
                                "Line: ")(line)(".")
                                .Flush();
                            return false;
                        }

                        if (bIsEOF || !raw_file_->sgets(buffer1.data(), 2048)) {
                            LogConsole()()("Error in signature for contract ")(
                                filename_.get())(
                                ": Unexpected EOF after Meta: .")
                                .Flush();
                            return false;
                        }

                        continue;
                    }
                }
                if (bContentMode) {
                    if (line.compare(0, 6, "Hash: ") == 0) {
                        LogDebug()()("Collecting message digest algorithm from "
                                     " contract header...")
                            .Flush();

                        UnallocatedCString strTemp = line.substr(6);
                        auto strHashType = String::Factory(strTemp.c_str());
                        strHashType->ConvertToUpperCase();

                        sig_hash_type_ =
                            crypto::HashingProvider::StringToHashType(
                                strHashType);

                        if (bIsEOF || !raw_file_->sgets(buffer1.data(), 2048)) {
                            LogConsole()()("Error in contract ")(
                                filename_.get())(
                                ": Unexpected EOF after Hash: .")
                                .Flush();
                            return false;
                        }

                        continue;
                    }
                }
            }
        }

        if (bSignatureMode) {
            if (nullptr == pSig) {
                LogConsole()()("Corrupted signature").Flush();

                return false;
            }
            line.append("\n");
            pSig->Concatenate(String::Factory(line));
        } else if (bContentMode) {
            line.append("\n");
            xml_unsigned_->Concatenate(String::Factory(line));
        }
    } while (!bIsEOF);

    if (!bHaveEnteredContentMode) {
        LogError()()(
            "Error in Contract::ParseRawFile: Found no BEGIN for signed "
            "content.")
            .Flush();
        return false;
    } else if (bContentMode) {
        LogError()()("Error in Contract::ParseRawFile: EOF while reading xml "
                     "content.")
            .Flush();
        return false;
    } else if (bSignatureMode) {
        LogError()()("Error in Contract::ParseRawFile: EOF while reading "
                     "signature.")
            .Flush();
        return false;
    } else if (!LoadContractXML()) {
        LogError()()("Error in Contract::ParseRawFile: Unable to load XML "
                     "portion of contract into memory.")
            .Flush();
        return false;
    } else if (crypto::HashType::Error == sig_hash_type_) {
        LogError()()("Failed to set hash type.").Flush();

        return false;
    } else {

        return true;
    }
}

// This function assumes that xml_unsigned_ is ready to be processed.
// This function only processes that portion of the contract.
auto Contract::LoadContractXML() -> bool
{
    std::int32_t retProcess = 0;

    if (!xml_unsigned_->Exists()) { return false; }

    xml_unsigned_->reset();

    auto* xml = irr::io::createIrrXMLReader(xml_unsigned_.get());
    assert_false(nullptr == xml, "Memory allocation issue with xml reader");
    std::unique_ptr<irr::io::IrrXMLReader> xmlAngel(xml);

    // parse the file until end reached
    while (xml->read()) {
        auto strNodeType = String::Factory();

        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
                strNodeType->Set("EXN_NONE");
                goto switch_log;
            case irr::io::EXN_COMMENT:
                strNodeType->Set("EXN_COMMENT");
                goto switch_log;
            case irr::io::EXN_ELEMENT_END:
                strNodeType->Set("EXN_ELEMENT_END");
                goto switch_log;
            case irr::io::EXN_CDATA:
                strNodeType->Set("EXN_CDATA");
                goto switch_log;

            switch_log:
                //                otErr << "SKIPPING %s element in
                // Contract::LoadContractXML: "
                //                              "type: %d, name: %s, value:
                //                              %s\n",
                //                              strNodeType.Get(),
                // xml->getNodeType(), xml->getNodeName(), xml->getNodeData());

                break;

            case irr::io::EXN_TEXT: {
                // unknown element type
                //                otErr << "SKIPPING unknown text element type
                //                in
                // Contract::LoadContractXML: %s, value: %s\n",
                //                              xml->getNodeName(),
                // xml->getNodeData());
            } break;
            case irr::io::EXN_ELEMENT: {
                retProcess = ProcessXMLNode(xml);

                // an error was returned. file format or whatever.
                if ((-1) == retProcess) {
                    LogError()()("(Cancelling this "
                                 "contract load; an error occurred).")
                        .Flush();
                    return false;
                }
                // No error, but also the node wasn't found...
                else if (0 == retProcess) {
                    // unknown element type
                    LogError()()("UNKNOWN element type in ")(
                        xml->getNodeName())(", value: ")(xml->getNodeData())(
                        ".")
                        .Flush();

                    LogError()()(xml_unsigned_.get())(".").Flush();
                }
                // else if 1 was returned, that means the node was processed.
            } break;
            case irr::io::EXN_UNKNOWN:
            default: {
                //                otErr << "SKIPPING (default case) element in
                // Contract::LoadContractXML: %d, value: %s\n",
                //                              xml->getNodeType(),
                // xml->getNodeData());
            }
                continue;
        }
    }

    return true;
}

// Make sure you escape any lines that begin with dashes using "- "
// So "---BEGIN " at the beginning of a line would change to: "- ---BEGIN"
// This function expects that's already been done.
// This function assumes there is only unsigned contents, and not a signed
// contract.
// This function is intended to PRODUCE said signed contract.
// NOTE: This function also assumes you already instantiated a contract
// of the proper type. For example, if it's an ServerContract, then you
// INSTANTIATED an ServerContract. Because if you tried to do this using
// an Contract but then the strContract was for an ServerContract, then
// this function will fail when it tries to "LoadContractFromString()" since it
// is not actually the proper type to handle those data members.
//
// Therefore I need to make an entirely different (but similar) function which
// can be used for signing a piece of unsigned XML where the actual contract
// type
// is unknown.
//
auto Contract::CreateContract(
    const String& strContract,
    const identity::Nym& theSigner,
    const PasswordPrompt& reason) -> bool
{
    Release();

    char cNewline = 0;  // this is about to contain a byte read from the end of
                        // the contract.
    const std::uint32_t lLength = strContract.GetLength();

    if ((3 > lLength) || !strContract.At(lLength - 1, cNewline)) {
        LogError()()(
            "Invalid input: Contract is less than 3 bytes "
            "std::int64_t, or unable to read a byte from the end where a "
            "newline is meant to be.")
            .Flush();
        return false;
    }

    // ADD a newline, if necessary.
    // (The -----BEGIN part needs to start on its OWN LINE...)
    //
    // If length is 10, then string goes from 0..9.
    // Null terminator will be at 10.
    // Therefore the final newline should be at 9.
    // Therefore if char_at_index[lLength-1] != '\n'
    // Concatenate one!

    if ('\n' == cNewline) {  // It already has a newline
        xml_unsigned_.get() = strContract;
    } else {
        xml_unsigned_->Set(strContract.Get());
    }

    // This function assumes that xml_unsigned_ is ready to be processed.
    // This function only processes that portion of the contract.
    //
    bool bLoaded = LoadContractXML();

    if (bLoaded) {

        // Add theSigner to the contract, if he's not already there.
        //
        if (nullptr == GetContractPublicNym()) {
            const bool bHasCredentials = (theSigner.HasCapability(
                identity::NymCapability::SIGN_MESSAGE));

            if (!bHasCredentials) {
                LogError()()("Signing nym has no credentials.").Flush();
                return false;
            } else  // theSigner has Credentials, so we'll add him to the
                    // contract.
            {
                auto pNym = api_.Wallet().Nym(theSigner.ID());
                if (nullptr == pNym) {
                    LogError()()("Failed to load signing nym.").Flush();
                    return false;
                }
                // Add pNym to the contract's internal list of nyms.
                nyms_["signer"] = pNym;
            }
        }
        // This re-writes the contract internally based on its data members,
        // similar to UpdateContents. (Except, specifically intended for the
        // initial creation of the contract.)
        // Since theSigner was just added, he will be included here now as well,
        // just prior to the actual signing below.
        //
        CreateContents();

        if (!SignContract(theSigner, reason)) {
            LogError()()("SignContract failed.").Flush();
            return false;
        }

        SaveContract();
        auto strTemp = String::Factory();
        SaveContractRaw(strTemp);

        if (LoadContractFromString(strTemp))  // The ultimate test is,
                                              // once
        {                                     // we've created the serialized
            auto NEW_ID =
                identifier::Generic{};    // string for this contract, is
            CalculateContractID(NEW_ID);  // to then load it up from that
                                          // string.
            id_ = NEW_ID;

            return true;
        }
    } else {
        LogError()()("LoadContractXML failed. strContract "
                     "contents: ")(strContract)(".")
            .Flush();
    }

    return false;
}

// Overrides of CreateContents call this in order to add some common internals.
//
void Contract::CreateInnerContents(Tag& parent)
{
    // CONDITIONS
    //
    if (!conditions_.empty()) {
        for (auto& it : conditions_) {
            UnallocatedCString str_condition_name = it.first;
            UnallocatedCString str_condition_value = it.second;

            TagPtr pTag(new Tag("condition", str_condition_value));
            pTag->add_attribute("name", str_condition_name);
            parent.add_tag(pTag);
        }
    }
    // CREDENTIALS
    //
    if (!nyms_.empty()) {
        // CREDENTIALS, based on NymID and Source, and credential IDs.
        for (auto& it : nyms_) {
            UnallocatedCString str_name = it.first;
            Nym_p pNym = it.second;
            assert_false(nullptr == pNym, "nullptr pseudonym pointer");

            if ("signer" == str_name) {
                assert_true(
                    pNym->HasCapability(identity::NymCapability::SIGN_MESSAGE));

                auto strNymID = String::Factory();
                pNym->GetIdentifier(strNymID);

                auto publicNym = proto::Nym{};
                assert_true(pNym->Internal().Serialize(publicNym));

                TagPtr pTag(new Tag(str_name));  // "signer"
                pTag->add_attribute("nymID", strNymID->Get());
                pTag->add_attribute(
                    "publicNym",
                    api_.Factory()
                        .InternalSession()
                        .Armored(publicNym, "PUBLIC NYM")
                        ->Get());

                parent.add_tag(pTag);
            }  // "signer"
        }
    }  // if (nyms_.size() > 0)
}

// Only used when first generating an asset or server contract.
// Meant for contracts which never change after that point.
// Otherwise does the same thing as UpdateContents.
// (But meant for a different purpose.)
// See ServerContract.cpp and OTUnitDefinition.cpp
//
void Contract::CreateContents()
{
    LogAbort()()("Contract::CreateContents should never be called, "
                 "but should be overridden. (In this case, it wasn't.)")
        .Abort();
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Contract::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("entity")) {
        entity_short_name_ =
            String::Factory(xml->getAttributeValue("shortname"));
        if (!name_->Exists()) {  // only set it if it's not already set,
                                 // since
            // the wallet may have already had a user label
            // set.
            name_ = entity_short_name_;  // name_ may later be changed
        }
        // again in
        // OTUnitDefinition::ProcessXMLNode

        entity_long_name_ = String::Factory(xml->getAttributeValue("longname"));
        entity_email_ = String::Factory(xml->getAttributeValue("email"));

        LogDetail()()("Loaded Entity, shortname: ")(entity_short_name_.get())(
            ", Longname: ")(entity_long_name_.get())(", email: ")(
            entity_email_.get())
            .Flush();

        return 1;
    } else if (strNodeName->Compare("condition")) {
        // todo security: potentially start ascii-encoding these.
        // (Are they still "human readable" if you can easily decode them?)
        //
        auto strConditionName = String::Factory();
        auto strConditionValue = String::Factory();

        strConditionName = String::Factory(xml->getAttributeValue("name"));

        if (!SkipToTextField(xml)) {
            LogDetail()()(
                "Failure: Unable to find expected "
                "text field for xml node named: ")(xml->getNodeName())(".")
                .Flush();
            return (-1);  // error condition
        }

        if (irr::io::EXN_TEXT == xml->getNodeType()) {
            strConditionValue = String::Factory(xml->getNodeData());
        } else {
            LogError()()(
                "Error in Contract::ProcessXMLNode: Condition without value: ")(
                strConditionName.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        // Add the conditions to a list in memory on this object.
        //
        conditions_.insert(std::pair<UnallocatedCString, UnallocatedCString>(
            strConditionName->Get(), strConditionValue->Get()));

        LogDetail()()("---- Loaded condition ")(strConditionName.get()).Flush();
        //        otWarn << "Loading condition \"%s\": %s----------(END
        // DATA)----------\n", strConditionName.Get(),
        //                strConditionValue.Get());

        return 1;
    } else if (strNodeName->Compare("signer")) {
        const auto strSignerNymID =
            String::Factory(xml->getAttributeValue("nymID"));

        if (!strSignerNymID->Exists()) {
            LogError()()("Error: Expected nymID attribute on signer element.")
                .Flush();
            return (-1);  // error condition
        }

        const auto nymId =
            api_.Factory().NymIDFromBase58(strSignerNymID->Bytes());
        const auto pNym = api_.Wallet().Nym(nymId);

        if (nullptr == pNym) {
            LogError()()("Failure loading signing nym.").Flush();

            return (-1);
        }
        // Add pNym to the contract's internal list of nyms.
        nyms_[strNodeName->Get() /*"signer"*/] = pNym;

        return 1;  // <==== Success!
    }
    return 0;
}
}  // namespace opentxs
