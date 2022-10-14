// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <iosfwd>

#include "internal/core/String.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/crypto/Signature.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace irr
{
namespace io
{
class IFileReadCallBack;
class IXMLBase;
template <class char_type, class super_class>
class IIrrXMLReader;

using IrrXMLReader = IIrrXMLReader<char, IXMLBase>;
}  // namespace io
}  // namespace irr

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
class Key;
}  // namespace asymmetric
}  // namespace crypto

namespace identity
{
class Nym;
}  // namespace identity

class Armored;
class PasswordPrompt;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class Contract
{
public:
    virtual auto CalculateContractID(identifier::Generic& newID) const -> void;
    auto GetFilename(String& strFilename) const -> void;
    auto GetIdentifier(String& theIdentifier) const -> void;
    virtual auto GetIdentifier(identifier::Generic& theIdentifier) const
        -> void;
    auto GetName(String& strName) const -> void { strName.Set(name_->Get()); }
    auto SaveContractRaw(String& strOutput) const -> bool;
    virtual auto VerifySignature(const identity::Nym& theNym) const -> bool;
    virtual auto VerifyWithKey(const crypto::asymmetric::Key& theKey) const
        -> bool;

    virtual auto LoadContractFromString(const String& theStr) -> bool;
    virtual auto SaveContract() -> bool;
    virtual auto SaveContract(const char* szFoldername, const char* szFilename)
        -> bool;
    auto ReleaseSignatures() -> void;
    virtual auto SignContract(
        const identity::Nym& theNym,
        const PasswordPrompt& reason) -> bool;
    auto SignWithKey(
        const crypto::asymmetric::Key& theKey,
        const PasswordPrompt& reason) -> bool;

    Contract() = delete;

    virtual ~Contract();

protected:
    using listOfSignatures = UnallocatedList<OTSignature>;

    const api::Session& api_;

    /** Contract name as shown in the wallet. */
    OTString name_;

    /** Foldername for this contract (nyms, contracts, accounts, etc) */
    OTString foldername_;

    /** Filename for this contract (usually an ID.) */
    OTString filename_;

    /** Hash of the contract, including signatures. (the "raw file") */
    identifier::Generic id_;

    /** The Unsigned Clear Text (XML contents without signatures.) */
    OTStringXML xml_unsigned_;

    /** The complete raw file including signatures. */
    OTString raw_file_;

    /** The Hash algorithm used for the signature */
    crypto::HashType sig_hash_type_;

    /** CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM */
    OTString contract_type_{String::Factory("CONTRACT")};

    /** The default behavior for a contract, though occasionally overridden, is
     * to contain its own public keys internally, located on standard XML tags.
     * So when we load a contract, we find its public key, and we verify its
     * signature with it. (It self-verifies!) I could be talking about an x509
     * as well, since people will need these to be revokable. The
     * Issuer/Server/etc URL will also be located within the contract, on a
     * standard tag, so by merely loading a contract, a wallet will know how to
     * connect to the relevant server, and the wallet will be able to encrypt
     * messages meant for that server to its public key without the normally
     * requisite key exchange. ==> THE TRADER HAS ASSURANCE THAT, IF HIS
     * OUT-MESSAGE IS ENCRYPTED, HE KNOWS THE MESSAGE CAN ONLY BE DECRYPTED BY
     * THE SAME PERSON WHO SIGNED THAT CONTRACT. */
    UnallocatedMap<UnallocatedCString, Nym_p> nyms_;

    /** The PGP signatures at the bottom of the XML file. */
    listOfSignatures list_signatures_;

    /** The version of this Contract file, in case the format changes in the
    future. */
    OTString version_{String::Factory("2.0")};

    // TODO: perhaps move these to a common ancestor for ServerContract and
    // OTUnitDefinition. Maybe call it OTHardContract (since it should never
    // change.)
    OTString entity_short_name_;
    OTString entity_long_name_;
    OTString entity_email_;

    /** The legal conditions, usually human-readable, on a contract. */
    String::Map conditions_;

    /** The XML file is in xml_unsigned_-> Load it from there into members here.
     */
    auto LoadContractXML() -> bool;

    /** parses raw_file_ into the various member variables. Separating these
     * into two steps allows us to load contracts from other sources besides
     * files. */
    auto ParseRawFile() -> bool;

    /** return -1 if error, 0 if nothing, and 1 if the node was processed. */
    virtual auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t;
    virtual void Release();
    void Release_Contract();

    /** This function is for those times when you already have the unsigned
     * version of the contract, and you have the signer, and you just want to
     * sign it and calculate its new ID from the finished result. */
    virtual auto CreateContract(
        const String& strContract,
        const identity::Nym& theSigner,
        const PasswordPrompt& reason) -> bool;

    void SetName(const String& strName) { name_ = strName; }
    auto GetContractType() const -> const String& { return contract_type_; }
    /** This function calls VerifyContractID, and if that checks out, then it
     * looks up the official "contract" key inside the contract by calling
     * GetContractPublicNym, and uses it to verify the signature on the
     * contract. So the contract is self-verifying. Right now only public keys
     * are supported, but soon contracts will also support x509 certs. */
    virtual auto VerifyContract() const -> bool;

    /** assumes filename_ is already set. Then it reads that file into a
     * string. Then it parses that string into the object. */
    virtual auto LoadContract() -> bool;
    auto LoadContract(const char* szFoldername, const char* szFilename) -> bool;

    /** fopens filename_ and reads it off the disk into raw_file_ */
    auto LoadContractRawFile() -> bool;

    /** data_folder/contracts/Contract-ID */
    auto SaveToContractFolder() -> bool;

    /** Takes the pre-existing XML contents (WITHOUT signatures) and re-writes
     * the Raw data, adding the pre-existing signatures along with new signature
     * bookends. */
    auto RewriteContract(String& strOutput) const -> bool;

    /** Writes the contract to a specific filename without changing member
     *  variables */
    auto WriteContract(
        const UnallocatedCString& folder,
        const UnallocatedCString& filename) const -> bool;

    /** Update the internal unsigned contents based on the member variables
     * default behavior does nothing. */
    virtual void UpdateContents(const PasswordPrompt& reason);

    /** Only used when first generating an asset or server contract. Meant for
     * contracts which never change after that point. Otherwise does the same
     * thing as UpdateContents. (But meant for a different purpose.) */
    virtual void CreateContents();

    /** Overrides of CreateContents call this in order to add some common
     * internals. */
    void CreateInnerContents(Tag& parent);

    /** Save the internal contents (xml_unsigned_) to an already-open file */
    virtual auto SaveContents(std::ofstream& ofs) const -> bool;

    /** Saves the entire contract to a file that's already open (like a wallet).
     */
    virtual auto SaveContractWallet(Tag& parent) const -> bool;

    virtual auto DisplayStatistics(String& strContents) const -> bool;

    /** Save xml_unsigned_ to a string that's passed in */
    virtual auto SaveContents(String& strContents) const -> bool;
    auto SignContractAuthent(
        const identity::Nym& theNym,
        const PasswordPrompt& reason) -> bool;
    auto SignContract(
        const identity::Nym& theNym,
        Signature& theSignature,
        const PasswordPrompt& reason) -> bool;

    /** Uses authentication key instead of signing key. */
    auto SignContractAuthent(
        const identity::Nym& theNym,
        Signature& theSignature,
        const PasswordPrompt& reason) -> bool;
    auto SignContract(
        const crypto::asymmetric::Key& theKey,
        Signature& theSignature,
        const crypto::HashType hashType,
        const PasswordPrompt& reason) -> bool;

    /** Calculates a hash of raw_file_ (the xml portion of the contract plus
    the signatures) and compares to id_ (supposedly the same. The ID is
    calculated by hashing the file.)

    Be careful here--asset contracts and server contracts can have this ID. But
    a class such as OTAccount will change in its datafile as the balance
    changes. Thus, the account must have a Unique ID that is NOT a hash of its
    file.

    This means it's important to have the ID function overridable for
    OTAccount... This also means that my wallet MUST be signed, and these files
    should have and encryption option also. Because if someone changes my
    account ID in the file, I have no way of re-calculating it from the account
    file, which changes! So my copies of the account file and wallet file are
    the only records of that account ID which is a giant std::int64_t number. */
    virtual auto VerifyContractID() const -> bool;
    virtual void CalculateAndSetContractID(identifier::Generic& newID);

    virtual auto VerifySigAuthent(const identity::Nym& theNym) const -> bool;
    auto VerifySignature(
        const identity::Nym& theNym,
        const Signature& theSignature) const -> bool;

    /** Uses authentication key instead of signing key. */
    auto VerifySigAuthent(
        const identity::Nym& theNym,
        const Signature& theSignature) const -> bool;
    auto VerifySignature(
        const crypto::asymmetric::Key& theKey,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool;
    auto GetContractPublicNym() const -> Nym_p;

    explicit Contract(const api::Session& api);
    explicit Contract(
        const api::Session& api,
        const String& name,
        const String& foldername,
        const String& filename,
        const String& strID);
    explicit Contract(
        const api::Session& api,
        const identifier::Generic& theID);
    explicit Contract(const api::Session& api, const String& strID);

private:
    auto SetIdentifier(const identifier::Generic& theID) -> void;
};
}  // namespace opentxs
