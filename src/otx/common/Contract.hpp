// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/HashType.hpp"
// IWYU pragma: no_include <irrxml/irrXML.hpp>

#pragma once

#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/XML.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/otx/common/Contract.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

namespace identity
{
class Nym;
}  // namespace identity

class PasswordPrompt;
class Tag;
}  // namespace opentxs

namespace opentxs::otx
{
class Contract::Imp : virtual public internal::Contract
{
public:
    auto GetIdentifier(String& theIdentifier) const noexcept -> void;
    virtual auto GetIdentifier(Identifier& theIdentifier) const noexcept
        -> void;
    auto GetName(String& strName) const noexcept -> void;
    auto VerifySignature(const identity::Nym& nym) const noexcept -> bool;
    auto VerifyWithKey(const crypto::key::Asymmetric& key) const noexcept
        -> bool;

    virtual auto SignContract(
        const identity::Nym& nym,
        const PasswordPrompt& reason) noexcept -> bool;
    auto SignWithKey(
        const crypto::key::Asymmetric& key,
        const PasswordPrompt& reason) noexcept -> bool;

    ~Imp() override;

protected:
    using listOfSignatures = std::pmr::list<OTSignature>;

    const api::Session& api_;
    OTString m_strName;
    OTString m_strFoldername;
    OTString m_strFilename;
    OTIdentifier m_ID;
    OTStringXML m_xmlUnsigned;
    OTString m_strRawFile;
    crypto::HashType m_strSigHashType;
    OTString m_strContractType;
    std::pmr::map<std::string, Nym_p> m_mapNyms;
    listOfSignatures m_listSignatures;
    OTString m_strVersion;
    OTString m_strEntityShortName;
    OTString m_strEntityLongName;
    OTString m_strEntityEmail;
    String::Map m_mapConditions;

    auto CalculateContractID(Identifier& newID) const noexcept -> void override;
    virtual auto DisplayStatistics(String& strContents) const noexcept -> bool;
    auto GetContractPublicNym() const noexcept -> Nym_p;
    auto GetContractType() const noexcept -> const String&;
    auto GetFilename(String& strFilename) const noexcept -> void final;
    auto RewriteContract(String& strOutput) const noexcept -> bool;
    virtual auto SaveContents(String& strContents) const noexcept -> bool;
    virtual auto SaveContents(std::ofstream& ofs) const noexcept -> bool;
    auto SaveContractRaw(String& strOutput) const noexcept -> bool final;
    virtual auto SaveContractWallet(Tag& parent) const noexcept -> bool;
    virtual auto VerifyContract() const noexcept -> bool;
    virtual auto VerifyContractID() const noexcept -> bool;
    virtual auto VerifySigAuthent(const identity::Nym& nym) const noexcept
        -> bool;
    auto VerifySigAuthent(
        const identity::Nym& nym,
        const Signature& theSignature) const noexcept -> bool;
    auto VerifySignature(
        const crypto::key::Asymmetric& key,
        const Signature& theSignature,
        const crypto::HashType hashType) const noexcept -> bool;
    auto VerifySignature(
        const identity::Nym& nym,
        const Signature& theSignature) const noexcept -> bool;
    auto WriteContract(const std::string& folder, const std::string& filename)
        const noexcept -> bool;

    virtual auto CalculateAndSetContractID(Identifier& newID) noexcept -> void;
    virtual auto CreateContents() noexcept -> void;
    auto CreateContract(
        const String& strContract,
        const identity::Nym& theSigner,
        const PasswordPrompt& reason) noexcept -> bool;
    auto CreateInnerContents(Tag& parent) noexcept -> void;
    virtual auto LoadContract() noexcept -> bool;
    auto LoadContract(const char* szFoldername, const char* szFilename) noexcept
        -> bool;
    auto LoadContractFromString(const String& theStr) noexcept -> bool override;
    auto LoadContractRawFile() noexcept -> bool;
    auto LoadContractXML() noexcept -> bool;
    auto ParseRawFile() noexcept -> bool;
    virtual auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) noexcept
        -> std::int32_t;
    virtual auto Release() noexcept -> void;
    auto Release_Contract() noexcept -> void;
    auto ReleaseSignatures() noexcept -> void final;
    auto SaveContract() noexcept -> bool override;
    auto SaveContract(const char* szFoldername, const char* szFilename) noexcept
        -> bool override;
    auto SaveToContractFolder() noexcept -> bool;
    auto SetName(const String& strName) noexcept -> void;
    auto SignContract(
        const crypto::key::Asymmetric& key,
        Signature& theSignature,
        const crypto::HashType hashType,
        const PasswordPrompt& reason) noexcept -> bool;
    auto SignContract(
        const identity::Nym& nym,
        Signature& theSignature,
        const PasswordPrompt& reason) noexcept -> bool;
    auto SignContractAuthent(
        const identity::Nym& nym,
        Signature& theSignature,
        const PasswordPrompt& reason) noexcept -> bool;
    auto SignContractAuthent(
        const identity::Nym& nym,
        const PasswordPrompt& reason) noexcept -> bool;
    virtual auto UpdateContents(const PasswordPrompt& reason) noexcept -> void;

    explicit Imp(const api::Session& api);
    explicit Imp(
        const api::Session& api,
        const String& name,
        const String& foldername,
        const String& filename,
        const String& strID);
    explicit Imp(const api::Session& api, const Identifier& theID);
    explicit Imp(const api::Session& api, const String& strID);

private:
    Imp() = delete;

    auto SetIdentifier(const Identifier& theID) noexcept -> void;
};
}  // namespace opentxs::otx
