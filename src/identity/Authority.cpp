// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/Authority.hpp"  // IWYU pragma: associated

#include <Authority.pb.h>  // IWYU pragma: keep
#include <ContactData.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <Signature.pb.h>
#include <VerificationItem.pb.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PasswordPrompt.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/ParameterType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Key.hpp"
#include "opentxs/identity/credential/Verification.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

namespace opentxs
{
template <typename Range, typename Function>
auto for_each(Range& range, Function f) -> Function
{
    return std::for_each(range.begin(), range.end(), f);
}

auto Factory::Authority(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const proto::KeyMode mode,
    const proto::Authority& serialized) -> identity::internal::Authority*
{
    using ReturnType = identity::implementation::Authority;

    try {

        return new ReturnType(api, parent, source, mode, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to create authority: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::Authority(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const crypto::Parameters& parameters,
    const VersionNumber nymVersion,
    const opentxs::PasswordPrompt& reason) -> identity::internal::Authority*
{
    using ReturnType = identity::implementation::Authority;

    try {

        return new ReturnType(
            api, parent, source, parameters, nymVersion, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to create authority: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::internal
{
auto Authority::NymToContactCredential(const VersionNumber nym) noexcept(false)
    -> VersionNumber
{
    using ReturnType = identity::implementation::Authority;

    return ReturnType::authority_to_contact_.at(
        ReturnType::nym_to_authority_.at(nym));
}
}  // namespace opentxs::identity::internal

namespace opentxs::identity::implementation
{
const VersionConversionMap Authority::authority_to_contact_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_primary_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_secondary_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};
const VersionConversionMap Authority::authority_to_verification_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 1},
};
const VersionConversionMap Authority::nym_to_authority_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};

Authority::Authority(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const proto::KeyMode mode,
    const Serialized& serialized) noexcept(false)
    : api_(api)
    , parent_(parent)
    , version_(serialized.version())
    , index_(serialized.index())
    , master_(load_master(api, *this, source, mode, serialized))
    , key_credentials_(load_child<credential::internal::Secondary>(
          api,
          source,
          *this,
          *master_,
          serialized,
          mode,
          proto::CREDROLE_CHILDKEY))
    , contact_credentials_(load_child<credential::internal::Contact>(
          api,
          source,
          *this,
          *master_,
          serialized,
          mode,
          proto::CREDROLE_CONTACT))
    , verification_credentials_(load_child<credential::internal::Verification>(
          api,
          source,
          *this,
          *master_,
          serialized,
          mode,
          proto::CREDROLE_VERIFY))
    , revoked_credentials_()
    , mode_(mode)
{
    if (false == bool(master_)) {
        throw std::runtime_error("Failed to create master credential");
    }

    const auto expected = api_.Factory().Internal().NymID(serialized.nymid());

    if (expected != parent_.Source().NymID()) {
        throw std::runtime_error("Invalid nym ID");
    }
}

Authority::Authority(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const crypto::Parameters& parameters,
    VersionNumber nymVersion,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : api_(api)
    , parent_(parent)
    , version_(nym_to_authority_.at(nymVersion))
    , index_(1)
    , master_(create_master(
          api,
          *this,
          source,
          nym_to_authority_.at(nymVersion),
          parameters,
          0,
          reason))
    , key_credentials_(create_child_credential(
          api,
          parameters,
          source,
          *master_,
          *this,
          version_,
          index_,
          reason))
    , contact_credentials_(create_contact_credental(
          api,
          parameters,
          source,
          *master_,
          *this,
          version_,
          reason))
    , verification_credentials_()
    , revoked_credentials_()
    , mode_(proto::KEYMODE_PRIVATE)
{
    if (false == bool(master_)) {
        throw std::runtime_error("Invalid master credential");
    }
}

auto Authority::AddChildKeyCredential(
    const crypto::Parameters& nymParameters,
    const opentxs::PasswordPrompt& reason) -> identifier::Generic
{
    auto output = identifier::Generic{};
    auto revisedParameters{nymParameters};
    revisedParameters.SetCredIndex(index_++);
    std::unique_ptr<credential::internal::Secondary> child{
        opentxs::Factory::Credential<credential::internal::Secondary>(
            api_,
            *this,
            parent_.Source(),
            *master_,
            authority_to_secondary_.at(version_),
            revisedParameters,
            proto::CREDROLE_CHILDKEY,
            reason)};

    if (!child) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to instantiate child key credential.")
            .Flush();

        return output;
    }

    output.Assign(child->ID());
    key_credentials_.emplace(output, child.release());

    return output;
}

auto Authority::AddContactCredential(
    const proto::ContactData& contactData,
    const opentxs::PasswordPrompt& reason) -> bool
{
    LogDetail()(OT_PRETTY_CLASS())("Adding a contact credential.").Flush();

    if (!master_) { return false; }

    auto parameters = crypto::Parameters{api_.Factory()};
    parameters.Internal().SetContactData(contactData);
    std::unique_ptr<credential::internal::Contact> credential{
        opentxs::Factory::Credential<credential::internal::Contact>(
            api_,
            *this,
            parent_.Source(),
            *master_,
            authority_to_contact_.at(version_),
            parameters,
            proto::CREDROLE_CONTACT,
            reason)};

    if (false == bool(credential)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct credential").Flush();

        return false;
    }

    auto id{credential->ID()};
    contact_credentials_.emplace(std::move(id), credential.release());
    const auto version =
        proto::RequiredAuthorityVersion(contactData.version(), version_);

    if (version > version_) { const_cast<VersionNumber&>(version_) = version; }

    return true;
}

auto Authority::AddVerificationCredential(
    const proto::VerificationSet& verificationSet,
    const opentxs::PasswordPrompt& reason) -> bool
{
    LogDetail()(OT_PRETTY_CLASS())("Adding a verification credential.").Flush();

    if (!master_) { return false; }

    auto parameters = crypto::Parameters{api_.Factory()};
    parameters.Internal().SetVerificationSet(verificationSet);
    std::unique_ptr<credential::internal::Verification> credential{
        opentxs::Factory::Credential<credential::internal::Verification>(
            api_,
            *this,
            parent_.Source(),
            *master_,
            authority_to_verification_.at(version_),
            parameters,
            proto::CREDROLE_VERIFY,
            reason)};

    if (false == bool(credential)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct credential").Flush();

        return false;
    }

    auto id{credential->ID()};
    verification_credentials_.emplace(std::move(id), credential.release());

    return true;
}

auto Authority::create_child_credential(
    const api::Session& api,
    const crypto::Parameters& parameters,
    const identity::Source& source,
    const credential::internal::Primary& master,
    internal::Authority& parent,
    const VersionNumber parentVersion,
    Bip32Index& index,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> KeyCredentialMap
{
    auto output = KeyCredentialMap{};
    output.emplace(create_key_credential(
        api, parameters, source, master, parent, parentVersion, index, reason));
    using Type = crypto::ParameterType;

    if (output.empty() && api::crypto::HaveSupport(Type::secp256k1)) {
        LogDetail()(OT_PRETTY_STATIC(Authority))(
            "Creating an secp256k1 child key credential.")
            .Flush();
        auto revised = parameters.ChangeType(crypto::ParameterType::secp256k1);
        output.emplace(create_key_credential(
            api,
            revised,
            source,
            master,
            parent,
            parentVersion,
            index,
            reason));
    }

    if (output.empty() && api::crypto::HaveSupport(Type::ed25519)) {
        LogDetail()(OT_PRETTY_STATIC(Authority))(
            "Creating an ed25519 child key credential.")
            .Flush();
        auto revised = parameters.ChangeType(crypto::ParameterType::ed25519);
        output.emplace(create_key_credential(
            api,
            revised,
            source,
            master,
            parent,
            parentVersion,
            index,
            reason));
    }

    if (output.empty() && api::crypto::HaveSupport(Type::rsa)) {
        LogDetail()(OT_PRETTY_STATIC(Authority))(
            "Creating an RSA child key credential.")
            .Flush();
        auto revised = parameters.ChangeType(crypto::ParameterType::rsa);
        output.emplace(create_key_credential(
            api,
            revised,
            source,
            master,
            parent,
            parentVersion,
            index,
            reason));
    }

    if (output.empty()) {
        throw std::runtime_error("Failed to generate child credentials");
    }

    return output;
}

auto Authority::create_contact_credental(
    const api::Session& api,
    const crypto::Parameters& parameters,
    const identity::Source& source,
    const credential::internal::Primary& master,
    internal::Authority& parent,
    const VersionNumber parentVersion,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    -> ContactCredentialMap
{
    auto output = ContactCredentialMap{};

    auto serialized = proto::ContactData{};
    if (parameters.Internal().GetContactData(serialized)) {
        auto pCredential = std::unique_ptr<credential::internal::Contact>{
            opentxs::Factory::Credential<credential::internal::Contact>(
                api,
                parent,
                source,
                master,
                authority_to_contact_.at(parentVersion),
                parameters,
                proto::CREDROLE_CONTACT,
                reason)};

        if (false == bool(pCredential)) {
            throw std::runtime_error("Failed to create contact credentials");
        }

        auto& credential = *pCredential;
        auto id = credential.ID();
        output.emplace(std::move(id), std::move(pCredential));
    }

    return output;
}

auto Authority::create_key_credential(
    const api::Session& api,
    const crypto::Parameters& parameters,
    const identity::Source& source,
    const credential::internal::Primary& master,
    internal::Authority& parent,
    const VersionNumber parentVersion,
    Bip32Index& index,
    const opentxs::PasswordPrompt& reason) noexcept(false) -> KeyCredentialItem
{
    auto output = std::pair<
        identifier::Generic,
        std::unique_ptr<credential::internal::Secondary>>{
        identifier::Generic{}, nullptr};
    auto& [id, pChild] = output;

    auto revised{parameters};
    revised.SetCredIndex(index++);
    pChild.reset(opentxs::Factory::Credential<credential::internal::Secondary>(
        api,
        parent,
        source,
        master,
        authority_to_secondary_.at(parentVersion),
        revised,
        proto::CREDROLE_CHILDKEY,
        reason));

    if (false == bool(pChild)) {
        throw std::runtime_error("Failed to create child credentials");
    }

    auto& child = *pChild;
    id = child.ID();

    return output;
}

auto Authority::create_master(
    const api::Session& api,
    identity::internal::Authority& owner,
    const identity::Source& source,
    const VersionNumber version,
    const crypto::Parameters& parameters,
    const Bip32Index index,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    -> std::unique_ptr<credential::internal::Primary>
{
    if (api::crypto::HaveHDKeys()) {
        if (0 != index) {
            throw std::runtime_error(
                "The master credential must be the first credential created");
        }

        if (0 != parameters.CredIndex()) {
            throw std::runtime_error("Invalid credential index");
        }
    }

    auto output = std::unique_ptr<credential::internal::Primary>{
        opentxs::Factory::PrimaryCredential(
            api,
            owner,
            source,
            parameters,
            authority_to_primary_.at(version),
            reason)};

    if (false == bool(output)) {
        throw std::runtime_error("Failed to instantiate master credential");
    }

    return output;
}

auto Authority::EncryptionTargets() const noexcept -> AuthorityKeys
{
    auto output = AuthorityKeys{GetMasterCredID(), {}};
    auto set = UnallocatedSet<crypto::asymmetric::Algorithm>{};
    auto& list = output.second;

    for (const auto& [id, pCredential] : key_credentials_) {
        const auto& cred = *pCredential;

        if (false == cred.hasCapability(NymCapability::ENCRYPT_MESSAGE)) {
            continue;
        }

        const auto& keypair =
            cred.GetKeypair(crypto::asymmetric::Role::Encrypt);
        set.emplace(keypair.GetPublicKey().Type());
    }

    std::ranges::copy(set, std::back_inserter(list));

    return output;
}

template <typename Type>
void Authority::extract_child(
    const api::Session& api,
    const identity::Source& source,
    internal::Authority& authority,
    const credential::internal::Primary& master,
    const credential::internal::Base::SerializedType& serialized,
    const proto::KeyMode mode,
    const proto::CredentialRole role,
    UnallocatedMap<identifier::Generic, std::unique_ptr<Type>>&
        map) noexcept(false)
{
    if (role != serialized.role()) { return; }

    bool valid = proto::Validate<proto::Credential>(
        serialized, VERBOSE, mode, role, true);

    if (false == valid) {
        throw std::runtime_error("Invalid serialized credential");
    }

    auto child = std::unique_ptr<Type>{opentxs::Factory::Credential<Type>(
        api, authority, source, master, serialized, mode, role)};

    if (false == bool(child)) {
        throw std::runtime_error("Failed to instantiate credential");
    }

    auto id{child->ID()};
    map.emplace(std::move(id), child.release());
}

auto Authority::get_keypair(
    const crypto::asymmetric::Algorithm type,
    const proto::KeyRole role,
    const String::List* plistRevokedIDs) const -> const crypto::key::Keypair&
{
    for (const auto& [id, pCredential] : key_credentials_) {
        OT_ASSERT(pCredential);

        const auto& credential = *pCredential;

        if (is_revoked(api_, id, plistRevokedIDs)) { continue; }

        try {
            return credential.GetKeypair(type, translate(role));
        } catch (...) {
            continue;
        }
    }

    throw std::out_of_range("no matching keypair");
}

auto Authority::get_secondary_credential(
    const identifier::Generic& strSubID,
    const String::List* plistRevokedIDs) const -> const credential::Base*
{
    if (is_revoked(api_, strSubID, plistRevokedIDs)) { return nullptr; }

    const auto it = key_credentials_.find(strSubID);

    if (key_credentials_.end() == it) { return nullptr; }

    return it->second.get();
}

auto Authority::GetContactData(proto::ContactData& contactData) const -> bool
{
    if (contact_credentials_.empty()) { return false; }

    // TODO handle more than one contact credential

    contact_credentials_.begin()->second->GetContactData(contactData);

    return true;
}

auto Authority::GetMasterCredID() const -> identifier::Generic
{
    OT_ASSERT(master_);

    return master_->ID();
}

auto Authority::GetAuthKeypair(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::key::Keypair&
{
    return get_keypair(keytype, proto::KEYROLE_AUTH, plistRevokedIDs);
}

auto Authority::GetEncrKeypair(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::key::Keypair&
{
    return get_keypair(keytype, proto::KEYROLE_ENCRYPT, plistRevokedIDs);
}

auto Authority::GetPublicAuthKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetAuthKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

auto Authority::GetPublicEncrKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetEncrKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

auto Authority::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const opentxs::Signature& theSignature,
    char cKeyType) const -> std::int32_t
{
    std::int32_t output{0};
    const auto getKeys = [&](const auto& item) -> void {
        output += item.second->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
    };

    for_each(key_credentials_, getKeys);

    return output;
}

auto Authority::GetPublicSignKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetSignKeypair(keytype, plistRevokedIDs).GetPublicKey();
}

auto Authority::GetPrivateAuthKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetAuthKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

auto Authority::GetPrivateEncrKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetEncrKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

auto Authority::GetPrivateSignKey(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::asymmetric::Key&
{
    return GetSignKeypair(keytype, plistRevokedIDs).GetPrivateKey();
}

auto Authority::GetSignKeypair(
    crypto::asymmetric::Algorithm keytype,
    const String::List* plistRevokedIDs) const -> const crypto::key::Keypair&
{
    return get_keypair(keytype, proto::KEYROLE_SIGN, plistRevokedIDs);
}

auto Authority::GetTagCredential(crypto::asymmetric::Algorithm type) const
    noexcept(false) -> const credential::Key&
{
    for (const auto& [id, pCredential] : key_credentials_) {
        const auto& cred = *pCredential;

        if (false == cred.hasCapability(NymCapability::ENCRYPT_MESSAGE)) {
            continue;
        }

        const auto& keypair =
            cred.GetKeypair(crypto::asymmetric::Role::Encrypt);

        if (type == keypair.GetPublicKey().Type()) { return cred; }
    }

    throw std::out_of_range("No matching credential");
}

auto Authority::GetVerificationSet(proto::VerificationSet& output) const -> bool
{
    if (verification_credentials_.empty()) { return false; }

    // TODO handle more than one verification credential

    verification_credentials_.begin()->second->GetVerificationSet(output);

    return true;
}

auto Authority::hasCapability(const NymCapability& capability) const -> bool
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            if (master_) { return master_->hasCapability(capability); }
        } break;
        case (NymCapability::SIGN_MESSAGE):
        case (NymCapability::ENCRYPT_MESSAGE):
        case (NymCapability::AUTHENTICATE_CONNECTION):
        default: {
            for (const auto& [id, pCredential] : key_credentials_) {
                OT_ASSERT(pCredential);

                const auto& credential = *pCredential;

                if (credential.hasCapability(capability)) { return true; }
            }
        }
    }

    return false;
}

auto Authority::is_revoked(
    const api::Session& api,
    const identifier::Generic& id,
    const String::List* plistRevokedIDs) -> bool
{
    if (nullptr == plistRevokedIDs) { return false; }

    const auto base58 = id.asBase58(api.Crypto());

    return std::ranges::find(*plistRevokedIDs, base58) !=
           plistRevokedIDs->end();
}

template <typename Type>
auto Authority::load_child(
    const api::Session& api,
    const identity::Source& source,
    internal::Authority& authority,
    const credential::internal::Primary& master,
    const Serialized& serialized,
    const proto::KeyMode mode,
    const proto::CredentialRole role) noexcept(false)
    -> UnallocatedMap<identifier::Generic, std::unique_ptr<Type>>
{
    auto output = UnallocatedMap<identifier::Generic, std::unique_ptr<Type>>{};

    if (proto::AUTHORITYMODE_INDEX == serialized.mode()) {
        for (const auto& it : serialized.activechildids()) {
            const auto id = api.Factory().Internal().Identifier(it);
            auto child = std::shared_ptr<proto::Credential>{};
            const auto loaded =
                api.Wallet().Internal().LoadCredential(id, child);

            if (false == loaded) {
                throw std::runtime_error("Failed to load credential");
            }

            extract_child(
                api, source, authority, master, *child, mode, role, output);
        }
    } else {
        for (const auto& it : serialized.activechildren()) {
            extract_child(
                api, source, authority, master, it, mode, role, output);
        }
    }

    return output;
}

auto Authority::LoadChildKeyCredential(const String& strSubID) -> bool
{

    OT_ASSERT(false == parent_.Source().NymID().empty());

    std::shared_ptr<proto::Credential> child;
    const auto id = api_.Factory().IdentifierFromBase58(strSubID.Bytes());
    const auto loaded = api_.Wallet().Internal().LoadCredential(id, child);

    if (!loaded) {
        LogError()(OT_PRETTY_CLASS())("Failure: Key Credential ")(
            strSubID)(" doesn't exist for Nym ")(
            parent_.Source().NymID(), api_.Crypto())
            .Flush();
        return false;
    }

    return LoadChildKeyCredential(*child);
}

auto Authority::LoadChildKeyCredential(const proto::Credential& serializedCred)
    -> bool
{
    bool validProto = proto::Validate<proto::Credential>(
        serializedCred, VERBOSE, mode_, proto::CREDROLE_ERROR, true);

    if (!validProto) {
        LogError()(OT_PRETTY_CLASS())(
            "Invalid serialized child key credential.")
            .Flush();
        return false;
    }

    if (proto::CREDROLE_MASTERKEY == serializedCred.role()) {
        LogError()(OT_PRETTY_CLASS())("Unexpected master credential.").Flush();

        return false;
    }

    switch (serializedCred.role()) {
        case proto::CREDROLE_CHILDKEY: {
            std::unique_ptr<credential::internal::Secondary> child{
                opentxs::Factory::Credential<credential::internal::Secondary>(
                    api_,
                    *this,
                    parent_.Source(),
                    *master_,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_CHILDKEY)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};

            key_credentials_.emplace(std::move(id), child.release());
        } break;
        case proto::CREDROLE_CONTACT: {
            std::unique_ptr<credential::internal::Contact> child{
                opentxs::Factory::Credential<credential::internal::Contact>(
                    api_,
                    *this,
                    parent_.Source(),
                    *master_,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_CONTACT)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};

            contact_credentials_.emplace(std::move(id), child.release());
        } break;
        case proto::CREDROLE_VERIFY: {
            std::unique_ptr<credential::internal::Verification> child{
                opentxs::Factory::Credential<
                    credential::internal::Verification>(
                    api_,
                    *this,
                    parent_.Source(),
                    *master_,
                    serializedCred,
                    mode_,
                    proto::CREDROLE_VERIFY)};

            if (false == bool(child)) { return false; }

            auto id{child->ID()};
            verification_credentials_.emplace(std::move(id), child.release());
        } break;
        case proto::CREDROLE_ERROR:
        case proto::CREDROLE_MASTERKEY:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid credential type").Flush();

            return false;
        }
    }

    return true;
}

auto Authority::load_master(
    const api::Session& api,
    identity::internal::Authority& owner,
    const identity::Source& source,
    const proto::KeyMode mode,
    const Serialized& serialized) noexcept(false)
    -> std::unique_ptr<credential::internal::Primary>
{
    auto output = std::unique_ptr<credential::internal::Primary>{};

    if (proto::AUTHORITYMODE_INDEX == serialized.mode()) {
        auto credential = std::shared_ptr<proto::Credential>{};
        const auto id =
            api.Factory().Internal().Identifier(serialized.masterid());

        if (!api.Wallet().Internal().LoadCredential(id, credential)) {
            throw std::runtime_error("Master credential does not exist");
        }

        OT_ASSERT(credential);

        output.reset(opentxs::Factory::PrimaryCredential(
            api, owner, source, *credential));
    } else {
        output.reset(opentxs::Factory::PrimaryCredential(
            api, owner, source, serialized.mastercredential()));
    }

    if (false == bool(output)) {
        throw std::runtime_error("Failed to instantiate master credential");
    }

    return output;
}

auto Authority::Params(const crypto::asymmetric::Algorithm type) const noexcept
    -> ReadView
{
    try {
        return GetTagCredential(type)
            .Internal()
            .asKey()
            .GetKeypair(type, crypto::asymmetric::Role::Encrypt)
            .GetPublicKey()
            .Internal()
            .Params();
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Invalid credential").Flush();

        return {};
    }
}

auto Authority::Path(proto::HDPath& output) const -> bool
{
    if (master_) {
        const auto found = master_->Path(output);

        if (found) { output.mutable_child()->RemoveLast(); }

        return found;
    }

    LogError()(OT_PRETTY_CLASS())("Master credential not instantiated.")
        .Flush();

    return false;
}

void Authority::RevokeContactCredentials(
    UnallocatedList<identifier::Generic>& output)
{
    const auto revoke = [&](const auto& item) -> void {
        output.push_back(item.first);
    };

    for_each(contact_credentials_, revoke);
    contact_credentials_.clear();
}

void Authority::RevokeVerificationCredentials(
    UnallocatedList<identifier::Generic>& output)
{
    const auto revoke = [&](const auto& item) -> void {
        output.push_back(item.first);
    };

    for_each(verification_credentials_, revoke);
    verification_credentials_.clear();
}

auto Authority::Serialize(
    Authority::Serialized& credSet,
    const CredentialIndexModeFlag mode) const -> bool
{
    credSet.set_version(version_);
    parent_.ID().Internal().Serialize(*credSet.mutable_nymid());
    GetMasterCredID().Internal().Serialize(*credSet.mutable_masterid());
    const auto add_active_id = [&](const auto& item) -> void {
        item.first.Internal().Serialize(*credSet.add_activechildids());
    };
    const auto add_revoked_id = [&](const auto& item) -> void {
        item.first.Internal().Serialize(*credSet.add_revokedchildids());
    };
    const auto add_active_child = [&](const auto& item) -> void {
        item.second->Serialize(
            *(credSet.add_activechildren()), AS_PUBLIC, WITH_SIGNATURES);
    };
    const auto add_revoked_child = [&](const auto& item) -> void {
        item.second->Serialize(
            *(credSet.add_revokedchildren()), AS_PUBLIC, WITH_SIGNATURES);
    };

    if (CREDENTIAL_INDEX_MODE_ONLY_IDS == mode) {
        if (proto::KEYMODE_PRIVATE == mode_) { credSet.set_index(index_); }

        credSet.set_mode(proto::AUTHORITYMODE_INDEX);
        for_each(key_credentials_, add_active_id);
        for_each(contact_credentials_, add_active_id);
        for_each(verification_credentials_, add_active_id);
        for_each(revoked_credentials_, add_revoked_id);
    } else {
        credSet.set_mode(proto::AUTHORITYMODE_FULL);
        master_->Serialize(
            *(credSet.mutable_mastercredential()), AS_PUBLIC, WITH_SIGNATURES);

        for_each(key_credentials_, add_active_child);
        for_each(contact_credentials_, add_active_child);
        for_each(verification_credentials_, add_active_child);
        for_each(revoked_credentials_, add_revoked_child);
    }

    return true;
}

auto Authority::Sign(
    const GetPreimage input,
    crypto::SignatureRole role,
    proto::Signature& output,
    const PasswordPrompt& reason) const -> bool
{
    return Sign(
        input,
        role,
        crypto::asymmetric::Role::Sign,
        crypto::HashType::Error,
        output,
        reason);
}

auto Authority::Sign(
    const GetPreimage input,
    crypto::SignatureRole role,
    crypto::HashType hash,
    proto::Signature& output,
    const PasswordPrompt& reason) const -> bool
{
    return Sign(
        input, role, crypto::asymmetric::Role::Sign, hash, output, reason);
}

auto Authority::Sign(
    const GetPreimage input,
    crypto::SignatureRole role,
    opentxs::crypto::asymmetric::Role key,
    proto::Signature& output,
    const PasswordPrompt& reason) const -> bool
{
    return Sign(input, role, key, crypto::HashType::Error, output, reason);
}

auto Authority::Sign(
    const GetPreimage input,
    crypto::SignatureRole role,
    opentxs::crypto::asymmetric::Role key,
    crypto::HashType hash,
    proto::Signature& output,
    const PasswordPrompt& reason) const -> bool
{
    switch (role) {
        case (crypto::SignatureRole::PublicCredential): {
            if (master_->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                return master_->Sign(input, role, output, reason, key, hash);
            }

            break;
        }
        case (crypto::SignatureRole::NymIDSource): {
            LogError()(": Credentials to be signed with a nym source can not "
                       "use this method.")
                .Flush();

            return false;
        }
        case (crypto::SignatureRole::PrivateCredential): {
            LogError()(": Private credential can not use this method.").Flush();

            return false;
        }
        default: {
            for (const auto& [id, pCredential] : key_credentials_) {
                OT_ASSERT(pCredential);

                const auto& credential = *pCredential;

                if (false ==
                    credential.hasCapability(NymCapability::SIGN_MESSAGE)) {
                    continue;
                }

                if (credential.Sign(input, role, output, reason, key, hash)) {

                    return true;
                }
            }
        }
    }

    return false;
}

auto Authority::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const opentxs::PasswordPrompt& reason) const -> bool
{
    for (const auto& [id, pCredential] : key_credentials_) {
        OT_ASSERT(pCredential);

        const auto& credential = *pCredential;

        if (false ==
            credential.hasCapability(NymCapability::AUTHENTICATE_CONNECTION)) {
            continue;
        }

        if (credential.TransportKey(publicKey, privateKey, reason)) {
            return true;
        }
    }

    LogError()(OT_PRETTY_CLASS())("No child credentials are capable of "
                                  "generating transport keys.")
        .Flush();

    return false;
}

auto Authority::Unlock(
    const crypto::asymmetric::Key& dhKey,
    const std::uint32_t tag,
    const crypto::asymmetric::Algorithm type,
    const crypto::symmetric::Key& key,
    PasswordPrompt& reason) const noexcept -> bool
{
    for (const auto& [id, pCredential] : key_credentials_) {
        const auto& cred = *pCredential;

        if (false == cred.hasCapability(NymCapability::ENCRYPT_MESSAGE)) {
            continue;
        }

        try {
            const auto& encryptKey =
                cred.GetKeypair(crypto::asymmetric::Role::Encrypt)
                    .GetPrivateKey();

            if (type != encryptKey.Type()) { continue; }

            auto testTag = std::uint32_t{};
            auto calculated = encryptKey.Internal().CalculateTag(
                dhKey, GetMasterCredID(), reason, testTag);

            if (false == calculated) {
                LogTrace()(OT_PRETTY_CLASS())("Unable to calculate tag")
                    .Flush();

                continue;
            }

            if (tag != testTag) {
                LogTrace()(OT_PRETTY_CLASS())(
                    "Session key not applicable to this nym")
                    .Flush();

                continue;
            }

            auto password = api_.Factory().Secret(0);
            calculated = encryptKey.Internal().CalculateSessionPassword(
                dhKey, reason, password);

            if (false == calculated) {
                LogError()(OT_PRETTY_CLASS())(
                    "Unable to calculate session password")
                    .Flush();

                continue;
            }

            reason.Internal().SetPassword(password);

            if (key.Unlock(reason)) { return true; }
        } catch (...) {
        }
    }

    return false;
}

template <typename Item>
auto Authority::validate_credential(const Item& item) const -> bool
{
    const auto& [id, pCredential] = item;

    if (nullptr == pCredential) {
        LogError()(OT_PRETTY_CLASS())("Null credential ")(id, api_.Crypto())(
            " in map")
            .Flush();

        return false;
    }

    const auto& credential = *pCredential;

    if (credential.Validate()) { return true; }

    LogError()(OT_PRETTY_CLASS())("Invalid credential ")(id, api_.Crypto())
        .Flush();

    return false;
}

auto Authority::Verify(const Data& plaintext, const proto::Signature& sig) const
    -> bool
{
    return Verify(plaintext, sig, crypto::asymmetric::Role::Sign);
}

auto Authority::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const crypto::asymmetric::Role key) const -> bool
{
    const auto signerID =
        api_.Factory().Internal().Identifier(sig.credentialid());

    if (signerID == GetMasterCredID()) {
        LogError()(OT_PRETTY_CLASS())(
            "Master credentials are only allowed to sign other credentials.")
            .Flush();

        return false;
    }

    const auto* credential = get_secondary_credential(signerID);

    if (nullptr == credential) {
        LogDebug()(OT_PRETTY_CLASS())(
            "This Authority does not contain the credential which produced "
            "the signature.")
            .Flush();

        return false;
    }

    return credential->Internal().Verify(plaintext, sig, key);
}

auto Authority::Verify(const proto::VerificationItem& item) const -> bool
{
    auto serialized = credential::Verification::SigningForm(item);
    auto& signature = *serialized.mutable_sig();
    proto::Signature signatureCopy;
    signatureCopy.CopyFrom(signature);
    signature.clear_signature();

    return Verify(
        api_.Factory().Internal().Data(serialized),
        signatureCopy,
        crypto::asymmetric::Role::Sign);
}

auto Authority::VerifyInternally() const -> bool
{
    if (false == bool(master_)) {
        LogConsole()(OT_PRETTY_CLASS())("Missing master credential.").Flush();
        return false;
    }

    if (false == master_->Validate()) {
        LogConsole()(OT_PRETTY_CLASS())("Master Credential failed to verify: ")(
            GetMasterCredID(),
            api_.Crypto())(" NymID: ")(parent_.Source().NymID(), api_.Crypto())
            .Flush();

        return false;
    }

    bool output{true};
    const auto validate = [&](const auto& item) -> void {
        output &= validate_credential(item);
    };

    for_each(key_credentials_, validate);
    for_each(contact_credentials_, validate);
    for_each(verification_credentials_, validate);

    return output;
}

auto Authority::WriteCredentials() const -> bool
{
    if (!master_->Save()) {
        LogError()(OT_PRETTY_CLASS())("Failed to save master credential.")
            .Flush();

        return false;
    }

    bool output{true};
    const auto save = [&](const auto& item) -> void {
        output &= item.second->Save();
    };

    for_each(key_credentials_, save);
    for_each(contact_credentials_, save);
    for_each(verification_credentials_, save);

    if (false == output) {
        LogError()(OT_PRETTY_CLASS())("Failed to save child credential.")
            .Flush();
    }

    return output;
}

Authority::~Authority() = default;
}  // namespace opentxs::identity::implementation
