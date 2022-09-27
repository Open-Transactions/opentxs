// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "1_Internal.hpp"    // IWYU pragma: associated
#include "identity/Nym.hpp"  // IWYU pragma: associated

#include <Authority.pb.h>
#include <ContactData.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <Nym.pb.h>
#include <NymIDSource.pb.h>
#include <Signature.pb.h>
#include <atomic>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "Proto.tpp"
#include "internal/api/crypto/Seed.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/identity/Nym.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/ContactData.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SignatureRole.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/HDIndex.hpp"

namespace opentxs
{
auto Factory::Nym(
    const api::Session& api,
    const crypto::Parameters& params,
    const identity::Type type,
    const UnallocatedCString name,
    const opentxs::PasswordPrompt& reason) -> identity::internal::Nym*
{
    using ReturnType = identity::implementation::Nym;

    if ((identity::CredentialType::Legacy == params.credentialType()) &&
        (identity::SourceType::Bip47 == params.SourceType())) {
        LogError()("opentxs::Factory::")(__func__)(": Invalid parameters")
            .Flush();

        return nullptr;
    }

    try {
        auto revised = ReturnType::normalize(api, params, reason);
        auto pSource = std::unique_ptr<identity::Source>{
            NymIDSource(api, revised, reason)};

        if (false == bool(pSource)) {
            LogError()("opentxs::Factory::")(__func__)(
                ": Failed to generate nym id source")
                .Flush();

            return nullptr;
        }

        if (identity::Type::invalid != type && !name.empty()) {
            const auto version =
                ReturnType::contact_credential_to_contact_data_version_.at(
                    identity::internal::Authority::NymToContactCredential(
                        identity::Nym::DefaultVersion));
            const auto blank = identity::wot::claim::Data{
                api,
                pSource->NymID().asBase58(api.Crypto()),
                version,
                version,
                identity::wot::claim::Data::SectionMap{}};
            const auto scope = blank.SetScope(NymToClaim(type), name);
            revised.Internal().SetContactData([&] {
                auto out = proto::ContactData{};
                scope.Serialize(out);

                return out;
            }());
        }

        return new ReturnType(api, revised, std::move(pSource), reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": Failed to create nym: ")(
            e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::Nym(
    const api::Session& api,
    const proto::Nym& serialized,
    const UnallocatedCString& alias) -> identity::internal::Nym*
{
    try {
        return new identity::implementation::Nym(api, serialized, alias);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to instantiate nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::Nym(
    const api::Session& api,
    const ReadView& view,
    const UnallocatedCString& alias) -> identity::internal::Nym*
{
    return Nym(api, proto::Factory<proto::Nym>(view), alias);
}

}  // namespace opentxs

namespace opentxs::identity
{
const VersionNumber Nym::DefaultVersion{6};
const VersionNumber Nym::MaxVersion{6};
}  // namespace opentxs::identity

namespace opentxs::identity::implementation
{
auto session_key_from_iv(
    const api::Session& api,
    const crypto::key::Asymmetric& signingKey,
    const Data& iv,
    const crypto::HashType hashType,
    opentxs::PasswordPrompt& reason) -> bool;

const VersionConversionMap Nym::akey_to_session_key_version_{
    {1, 1},
    {2, 1},
};
const VersionConversionMap Nym::contact_credential_to_contact_data_version_{
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 6},
};

Nym::Nym(
    const api::Session& api,
    crypto::Parameters& params,
    std::unique_ptr<const identity::Source> source,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : api_(api)
    , source_p_(std::move(source))
    , source_(*source_p_)
    , id_(source_.NymID())
    , mode_(proto::NYM_PRIVATE)
    , version_(DefaultVersion)
    , index_(1)
    , alias_()
    , revision_(0)
    , contact_data_(nullptr)
    , active_(create_authority(api_, *this, source_, version_, params, reason))
    , revoked_sets_()
    , list_revoked_ids_()
{
    OT_ASSERT(id_.Type() == identifier::Type::nym);

    if (false == bool(source_p_)) {
        throw std::runtime_error("Invalid nym id source");
    }
}

Nym::Nym(
    const api::Session& api,
    const proto::Nym& serialized,
    const UnallocatedCString& alias) noexcept(false)
    : api_(api)
    , source_p_(opentxs::Factory::NymIDSource(api, serialized.source()))
    , source_(*source_p_)
    , id_(source_.NymID())
    , mode_(serialized.mode())
    , version_(serialized.version())
    , index_(serialized.index())
    , alias_(alias)
    , revision_(serialized.revision())
    , contact_data_(nullptr)
    , active_(load_authorities(api_, *this, source_, serialized))
    , revoked_sets_()
    , list_revoked_ids_(
          load_revoked(api_, *this, source_, serialized, revoked_sets_))
{
    OT_ASSERT(id_.Type() == identifier::Type::nym);

    if (false == bool(source_p_)) {
        throw std::runtime_error("Invalid nym id source");
    }
}

auto Nym::add_contact_credential(
    const eLock& lock,
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason) -> bool
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddContactCredential(data, reason);

                break;
            }
        }
    }

    return added;
}

auto Nym::add_verification_credential(
    const eLock& lock,
    const proto::VerificationSet& data,
    const opentxs::PasswordPrompt& reason) -> bool
{
    OT_ASSERT(verify_lock(lock));

    bool added = false;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->hasCapability(NymCapability::SIGN_CHILDCRED)) {
                added = it.second->AddVerificationCredential(data, reason);

                break;
            }
        }
    }

    return added;
}

auto Nym::AddChildKeyCredential(
    const identifier::Generic& masterID,
    const crypto::Parameters& nymParameters,
    const opentxs::PasswordPrompt& reason) -> UnallocatedCString
{
    eLock lock(shared_lock_);

    UnallocatedCString output;
    auto it = active_.find(masterID);
    const bool noMaster = (it == active_.end());

    if (noMaster) {
        LogError()(OT_PRETTY_CLASS())("Master ID not found.").Flush();

        return output;
    }

    if (it->second) {
        output = it->second->AddChildKeyCredential(nymParameters, reason);
    }

    return output;
}

auto Nym::AddClaim(const Claim& claim, const opentxs::PasswordPrompt& reason)
    -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(contact_data_->AddItem(claim)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddContract(
    const identifier::UnitDefinition& instrumentDefinitionID,
    const UnitType currency,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active) -> bool
{
    const UnallocatedCString id(instrumentDefinitionID.asBase58(api_.Crypto()));

    if (id.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(
        contact_data_->AddContract(id, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddEmail(
    const UnallocatedCString& value,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(
        new wot::claim::Data(contact_data_->AddEmail(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddPaymentCode(
    const opentxs::PaymentCode& code,
    const UnitType currency,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active) -> bool
{
    const auto paymentCode = code.asBase58();

    if (paymentCode.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(
        contact_data_->AddPaymentCode(paymentCode, currency, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddPhoneNumber(
    const UnallocatedCString& value,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(
        contact_data_->AddPhoneNumber(value, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddPreferredOTServer(
    const identifier::Generic& id,
    const opentxs::PasswordPrompt& reason,
    const bool primary) -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(
        new wot::claim::Data(contact_data_->AddPreferredOTServer(id, primary)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::AddSocialMediaProfile(
    const UnallocatedCString& value,
    const wot::claim::ClaimType type,
    const opentxs::PasswordPrompt& reason,
    const bool primary,
    const bool active) -> bool
{
    if (value.empty()) { return false; }

    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(
        contact_data_->AddSocialMediaProfile(value, type, primary, active)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::Alias() const -> UnallocatedCString { return alias_; }

auto Nym::Serialize(AllocateOutput destination) const -> bool
{
    auto serialized = proto::Nym{};
    if (false == Serialize(serialized)) { return false; }

    write(serialized, destination);

    return true;
}

auto Nym::Serialize(Nym::Serialized& serialized) const -> bool
{
    if (false == SerializeCredentialIndex(serialized, Mode::Full)) {
        return false;
    }

    return true;
}

auto Nym::at(const std::size_t& index) const noexcept(false)
    -> const Nym::value_type&
{
    for (auto i{active_.cbegin()}; i != active_.cend(); ++i) {
        if (static_cast<std::size_t>(std::distance(active_.cbegin(), i)) ==
            index) {
            return *i->second;
        }
    }

    throw std::out_of_range("Invalid authority index");
}

auto Nym::BestEmail() const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestEmail();
}

auto Nym::BestPhoneNumber() const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestPhoneNumber();
}

auto Nym::BestSocialMediaProfile(const wot::claim::ClaimType type) const
    -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->BestSocialMediaProfile(type);
}

auto Nym::Claims() const -> const wot::claim::Data&
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return *contact_data_;
}

auto Nym::CompareID(const identity::Nym& rhs) const -> bool
{
    auto lock = sLock{shared_lock_};

    return rhs.CompareID(id_);
}

auto Nym::CompareID(const identifier::Nym& rhs) const -> bool
{
    auto lock = sLock{shared_lock_};

    return id_ == rhs;
}

auto Nym::ContactCredentialVersion() const -> VersionNumber
{
    // TODO support multiple authorities
    OT_ASSERT(0 < active_.size());

    return active_.cbegin()->second->ContactCredentialVersion();
}

auto Nym::Contracts(const UnitType currency, const bool onlyActive) const
    -> UnallocatedSet<identifier::Generic>
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->Contracts(currency, onlyActive);
}

auto Nym::create_authority(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const VersionNumber version,
    const crypto::Parameters& params,
    const PasswordPrompt& reason) noexcept(false) -> CredentialMap
{
    auto output = CredentialMap{};
    auto pAuthority = std::unique_ptr<identity::internal::Authority>(
        opentxs::Factory::Authority(
            api, parent, source, params, version, reason));

    if (false == bool(pAuthority)) {
        throw std::runtime_error("Failed to create nym authority");
    }

    auto& authority = *pAuthority;
    auto id{authority.GetMasterCredID()};
    output.emplace(std::move(id), std::move(pAuthority));

    return output;
}

auto Nym::DeleteClaim(
    const identifier::Generic& id,
    const opentxs::PasswordPrompt& reason) -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(new wot::claim::Data(contact_data_->Delete(id)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::EmailAddresses(bool active) const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->EmailAddresses(active);
}

auto Nym::EncryptionTargets() const noexcept -> NymKeys
{
    auto lock = sLock{shared_lock_};
    auto output = NymKeys{id_, {}};

    for (const auto& [id, pAuthority] : active_) {
        const auto& authority = *pAuthority;

        if (authority.hasCapability(NymCapability::ENCRYPT_MESSAGE)) {
            output.second.emplace_back(authority.EncryptionTargets());
        }
    }

    return output;
}

void Nym::GetIdentifier(identifier::Nym& theIdentifier) const
{
    auto lock = sLock{shared_lock_};

    theIdentifier.Assign(id_);
}

// sets argument based on internal member
void Nym::GetIdentifier(String& theIdentifier) const
{
    auto lock = sLock{shared_lock_};

    id_.GetString(api_.Crypto(), theIdentifier);
}

template <typename T>
auto Nym::get_private_auth_key(
    const T& lock,
    crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));
    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPrivateAuthKey(
        keytype, &list_revoked_ids_);  // success
}

auto Nym::GetPrivateAuthKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    return get_private_auth_key(lock, keytype);
}

auto Nym::GetPrivateEncrKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPrivateEncrKey(
        keytype,
        &list_revoked_ids_);  // success
}

auto Nym::GetPrivateSignKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    return get_private_sign_key(lock, keytype);
}

template <typename T>
auto Nym::get_private_sign_key(
    const T& lock,
    crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPrivateSignKey(
        keytype,
        &list_revoked_ids_);  // success
}

template <typename T>
auto Nym::get_public_sign_key(
    const T& lock,
    crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    OT_ASSERT(!active_.empty());

    OT_ASSERT(verify_lock(lock));

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPublicSignKey(
        keytype,
        &list_revoked_ids_);  // success
}

auto Nym::GetPublicAuthKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};

    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPublicAuthKey(
        keytype,
        &list_revoked_ids_);  // success
}

auto Nym::GetPublicEncrKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    OT_ASSERT(!active_.empty());

    const identity::Authority* pCredential{nullptr};
    for (const auto& it : active_) {
        // Todo: If we have some criteria, such as which master or
        // child credential
        // is currently being employed by the user, we'll use that here to
        // skip
        // through this loop until we find the right one. Until then, I'm
        // just
        // going to return the first one that's valid (not null).

        pCredential = it.second.get();
        if (nullptr != pCredential) { break; }
    }
    if (nullptr == pCredential) { OT_FAIL; }

    return pCredential->GetPublicEncrKey(
        keytype,
        &list_revoked_ids_);  // success
}

// This is being called by:
// Contract::VerifySignature(const Nym& theNym, const Signature&
// theSignature, PasswordPrompt * pPWData=nullptr)
//
// Note: Need to change Contract::VerifySignature so that it checks all of
// these keys when verifying.
//
// OT uses the signature's metadata to narrow down its search for the correct
// public key.
// Return value is the count of public keys found that matched the metadata on
// the signature.
auto Nym::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
    char cKeyType) const -> std::int32_t
{
    // Unfortunately, theSignature can only narrow the search down (there may be
    // multiple results.)
    std::int32_t nCount = 0;

    auto lock = sLock{shared_lock_};

    for (const auto& it : active_) {
        const identity::Authority* pCredential = it.second.get();
        OT_ASSERT(nullptr != pCredential);

        const std::int32_t nTempCount = pCredential->GetPublicKeysBySignature(
            listOutput, theSignature, cKeyType);
        nCount += nTempCount;
    }

    return nCount;
}

auto Nym::GetPublicSignKey(crypto::key::asymmetric::Algorithm keytype) const
    -> const crypto::key::Asymmetric&
{
    auto lock = sLock{shared_lock_};

    return get_public_sign_key(lock, keytype);
}

auto Nym::has_capability(const eLock& lock, const NymCapability& capability)
    const -> bool
{
    OT_ASSERT(verify_lock(lock));

    for (const auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const identity::Authority& credSet = *it.second;

            if (credSet.hasCapability(capability)) { return true; }
        }
    }

    return false;
}

auto Nym::HasCapability(const NymCapability& capability) const -> bool
{
    eLock lock(shared_lock_);

    return has_capability(lock, capability);
}

auto Nym::HasPath() const -> bool
{
    auto path = proto::HDPath{};

    if (false == Path(path)) { return false; }

    return true;
}

void Nym::init_claims(const eLock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    const auto nymID{id_.asBase58(api_.Crypto())};
    const auto dataVersion = ContactDataVersion();
    contact_data_ = std::make_unique<wot::claim::Data>(
        api_, nymID, dataVersion, dataVersion, wot::claim::Data::SectionMap());

    OT_ASSERT(contact_data_);

    for (const auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        const auto& credSet = *it.second;
        auto serialized = proto::ContactData{};
        if (credSet.GetContactData(serialized)) {
            OT_ASSERT(
                proto::Validate(serialized, VERBOSE, proto::ClaimType::Normal));

            wot::claim::Data claimCred(api_, nymID, dataVersion, serialized);
            // NOLINTNEXTLINE(modernize-make-unique)
            contact_data_.reset(
                new wot::claim::Data(*contact_data_ + claimCred));
        }
    }

    OT_ASSERT(contact_data_);
}

auto Nym::load_authorities(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const Serialized& serialized) noexcept(false) -> CredentialMap
{
    auto output = CredentialMap{};

    if (false == proto::Validate<proto::Nym>(serialized, VERBOSE)) {
        throw std::runtime_error("Invalid serialized nym");
    }

    const auto mode = (proto::NYM_PRIVATE == serialized.mode())
                          ? proto::KEYMODE_PRIVATE
                          : proto::KEYMODE_PUBLIC;

    for (const auto& it : serialized.activecredentials()) {
        auto pCandidate = std::unique_ptr<identity::internal::Authority>{
            opentxs::Factory::Authority(api, parent, source, mode, it)};

        if (false == bool(pCandidate)) {
            throw std::runtime_error("Failed to instantiate authority");
        }

        const auto& candidate = *pCandidate;
        auto id{candidate.GetMasterCredID()};
        output.emplace(std::move(id), std::move(pCandidate));
    }

    return output;
}

auto Nym::load_revoked(
    const api::Session& api,
    const identity::Nym& parent,
    const identity::Source& source,
    const Serialized& serialized,
    CredentialMap& revoked) noexcept(false) -> String::List
{
    auto output = String::List{};

    if (!opentxs::operator==(Serialized::default_instance(), serialized)) {
        const auto mode = (proto::NYM_PRIVATE == serialized.mode())
                              ? proto::KEYMODE_PRIVATE
                              : proto::KEYMODE_PUBLIC;

        for (const auto& it : serialized.revokedcredentials()) {
            auto pCandidate = std::unique_ptr<identity::internal::Authority>{
                opentxs::Factory::Authority(api, parent, source, mode, it)};

            if (false == bool(pCandidate)) {
                throw std::runtime_error("Failed to instantiate authority");
            }

            const auto& candidate = *pCandidate;
            auto id{candidate.GetMasterCredID()};
            output.push_back(id.asBase58(api.Crypto()));
            revoked.emplace(std::move(id), std::move(pCandidate));
        }
    }

    return output;
}

auto Nym::Name() const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    UnallocatedCString output = contact_data_->Name();

    if (false == output.empty()) { return output; }

    return alias_;
}

auto Nym::normalize(
    const api::Session& api,
    const crypto::Parameters& in,
    const PasswordPrompt& reason) noexcept(false) -> crypto::Parameters
{
    auto output{in};

    if (identity::CredentialType::HD == in.credentialType()) {
        if (false == api::crypto::HaveHDKeys()) {
            throw std::runtime_error(
                "opentxs compiled without hd credential support");
        }

        const auto& seeds = api.Crypto().Seed().Internal();
        output.SetCredset(0);
        auto nymIndex = Bip32Index{0};
        auto fingerprint = in.Seed();
        auto style = in.SeedStyle();
        auto lang = in.SeedLanguage();

        if (fingerprint.empty()) {
            seeds.GetOrCreateDefaultSeed(
                fingerprint, style, lang, nymIndex, in.SeedStrength(), reason);
        }

        auto seed = seeds.GetSeed(fingerprint, nymIndex, reason);
        const auto defaultIndex = in.UseAutoIndex();

        if (false == defaultIndex) {
            LogDetail()(OT_PRETTY_STATIC(Nym))(
                "Re-creating nym at specified path.")
                .Flush();

            nymIndex = in.Nym();
        }

        static constexpr auto maxIndex =
            std::numeric_limits<std::int32_t>::max() - 1;

        if (nymIndex >= static_cast<Bip32Index>(maxIndex)) {
            throw std::runtime_error(
                "Requested seed has already generated maximum number of nyms");
        }

        const auto newIndex = static_cast<std::int32_t>(nymIndex) + 1;
        seeds.UpdateIndex(fingerprint, newIndex, reason);
        output.SetEntropy(seed);
        output.SetSeed(fingerprint);
        output.SetNym(nymIndex);
    }

    return output;
}

auto Nym::path(const sLock& lock, proto::HDPath& output) const -> bool
{
    for (const auto& it : active_) {
        OT_ASSERT(nullptr != it.second);
        const auto& authority = *it.second;

        if (authority.Path(output)) {
            output.mutable_child()->RemoveLast();

            return true;
        }
    }

    LogError()(OT_PRETTY_CLASS())("No authority contains a path.").Flush();

    return false;
}

auto Nym::Path(proto::HDPath& output) const -> bool
{
    auto lock = sLock{shared_lock_};

    return path(lock, output);
}

auto Nym::PathRoot() const -> const UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    auto proto = proto::HDPath{};
    if (false == path(lock, proto)) { return ""; }
    return proto.root();
}

auto Nym::PathChildSize() const -> int
{
    auto lock = sLock{shared_lock_};

    auto proto = proto::HDPath{};
    if (false == path(lock, proto)) { return 0; }
    return proto.child_size();
}

auto Nym::PathChild(int index) const -> std::uint32_t
{
    auto lock = sLock{shared_lock_};

    auto proto = proto::HDPath{};
    if (false == path(lock, proto)) { return 0; }
    return proto.child(index);
}

auto Nym::PaymentCode() const -> UnallocatedCString
{
    if (identity::SourceType::Bip47 != source_.Type()) { return ""; }

    auto serialized = proto::NymIDSource{};
    if (false == source_.Serialize(serialized)) { return ""; }

    auto paymentCode =
        api_.Factory().InternalSession().PaymentCode(serialized.paymentcode());

    return paymentCode.asBase58();
}

auto Nym::PaymentCodePath(AllocateOutput destination) const -> bool
{
    auto path = proto::HDPath{};
    if (false == PaymentCodePath(path)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to serialize payment code path to HDPath.")
            .Flush();

        return false;
    }

    write(path, destination);

    return true;
}

auto Nym::PaymentCodePath(proto::HDPath& output) const -> bool
{
    auto base = proto::HDPath{};

    if (false == Path(base)) { return false; }

    if (2 != base.child().size()) { return false; }

    static const auto expected =
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED};

    if (expected != base.child(0)) { return false; }

    output.set_version(base.version());
    output.set_root(base.root());
    output.add_child(HDIndex{Bip43Purpose::PAYCODE, Bip32Child::HARDENED});
    output.add_child(HDIndex{Bip44Type::BITCOIN, Bip32Child::HARDENED});
    output.add_child(base.child(1));

    return true;
}

auto Nym::PhoneNumbers(bool active) const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->PhoneNumbers(active);
}

auto Nym::Revision() const -> std::uint64_t { return revision_.load(); }

void Nym::revoke_contact_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    UnallocatedList<UnallocatedCString> revokedIDs;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            it.second->RevokeContactCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { list_revoked_ids_.push_back(it); }
}

void Nym::revoke_verification_credentials(const eLock& lock)
{
    OT_ASSERT(verify_lock(lock));

    UnallocatedList<UnallocatedCString> revokedIDs;

    for (auto& it : active_) {
        if (nullptr != it.second) {
            it.second->RevokeVerificationCredentials(revokedIDs);
        }
    }

    for (auto& it : revokedIDs) { list_revoked_ids_.push_back(it); }
}

auto Nym::SerializeCredentialIndex(AllocateOutput destination, const Mode mode)
    const -> bool
{
    auto serialized = proto::Nym{};
    if (false == SerializeCredentialIndex(serialized, mode)) { return false; }

    return write(serialized, destination);
}

auto Nym::SerializeCredentialIndex(Serialized& index, const Mode mode) const
    -> bool
{
    auto lock = sLock{shared_lock_};
    index.set_version(version_);
    index.set_nymid(id_.asBase58(api_.Crypto()));

    if (Mode::Abbreviated == mode) {
        index.set_mode(mode_);

        if (proto::NYM_PRIVATE == mode_) { index.set_index(index_); }
    } else {
        index.set_mode(proto::NYM_PUBLIC);
    }

    index.set_revision(revision_.load());

    if (false == source_.Serialize(*(index.mutable_source()))) { return false; }

    for (const auto& it : active_) {
        if (nullptr != it.second) {
            auto credset = proto::Authority{};
            if (false ==
                it.second->Serialize(credset, static_cast<bool>(mode))) {
                return false;
            }
            auto* pCredSet = index.add_activecredentials();
            *pCredSet = credset;
            pCredSet = nullptr;
        }
    }

    for (const auto& it : revoked_sets_) {
        if (nullptr != it.second) {
            auto credset = proto::Authority{};
            if (false ==
                it.second->Serialize(credset, static_cast<bool>(mode))) {
                return false;
            }
            auto* pCredSet = index.add_revokedcredentials();
            *pCredSet = credset;
            pCredSet = nullptr;
        }
    }

    return true;
}

void Nym::SerializeNymIDSource(Tag& parent) const
{
    // We encode these before storing.
    TagPtr pTag(new Tag("nymIDSource", source_.asString()->Get()));
    const auto description = source_.Description();

    if (description->Exists()) {
        auto ascDescription = Armored::Factory();
        ascDescription->SetString(
            description,
            false);  // bLineBreaks=true by default.

        pTag->add_attribute("Description", ascDescription->Get());
    }

    parent.add_tag(pTag);
}

auto Nym::set_contact_data(
    const eLock& lock,
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason) -> bool
{
    OT_ASSERT(verify_lock(lock));

    auto version = proto::NymRequiredVersion(data.version(), version_);

    if ((0 == version) || version > MaxVersion) {
        LogError()(OT_PRETTY_CLASS())(
            "Contact data version not supported by this nym.")
            .Flush();

        return false;
    }

    if (false == has_capability(lock, NymCapability::SIGN_CHILDCRED)) {
        LogError()(OT_PRETTY_CLASS())("This nym can not be modified.").Flush();

        return false;
    }

    if (false == proto::Validate(data, VERBOSE, proto::ClaimType::Normal)) {
        LogError()(OT_PRETTY_CLASS())("Invalid contact data.").Flush();

        return false;
    }

    revoke_contact_credentials(lock);

    if (add_contact_credential(lock, data, reason)) {

        return update_nym(lock, version, reason);
    }

    return false;
}

void Nym::SetAlias(const UnallocatedCString& alias)
{
    eLock lock(shared_lock_);

    alias_ = alias;
    revision_++;
}

auto Nym::SetCommonName(
    const UnallocatedCString& name,
    const opentxs::PasswordPrompt& reason) -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    // NOLINTNEXTLINE(modernize-make-unique)
    contact_data_.reset(
        new wot::claim::Data(contact_data_->SetCommonName(name)));

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::SetContactData(
    const ReadView bytes,
    const opentxs::PasswordPrompt& reason) -> bool
{
    return SetContactData(proto::Factory<proto::ContactData>(bytes), reason);
}

auto Nym::SetContactData(
    const proto::ContactData& data,
    const opentxs::PasswordPrompt& reason) -> bool
{
    eLock lock(shared_lock_);
    contact_data_ = std::make_unique<wot::claim::Data>(
        api_, id_.asBase58(api_.Crypto()), ContactDataVersion(), data);

    return set_contact_data(lock, data, reason);
}

auto Nym::SetScope(
    const wot::claim::ClaimType type,
    const UnallocatedCString& name,
    const opentxs::PasswordPrompt& reason,
    const bool primary) -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    if (wot::claim::ClaimType::Unknown != contact_data_->Type()) {
        // NOLINTNEXTLINE(modernize-make-unique)
        contact_data_.reset(
            new wot::claim::Data(contact_data_->SetName(name, primary)));
    } else {
        // NOLINTNEXTLINE(modernize-make-unique)
        contact_data_.reset(
            new wot::claim::Data(contact_data_->SetScope(type, name)));
    }

    OT_ASSERT(contact_data_);

    return set_contact_data(
        lock,
        [&] {
            auto out = proto::ContactData{};
            contact_data_->Serialize(out);
            return out;
        }(),
        reason);
}

auto Nym::Sign(
    const ProtobufType& input,
    const crypto::SignatureRole role,
    proto::Signature& signature,
    const opentxs::PasswordPrompt& reason,
    const crypto::HashType hash) const -> bool
{
    auto lock = sLock{shared_lock_};

    bool haveSig = false;

    auto preimage = [&input]() -> UnallocatedCString {
        return proto::ToString(input);
    };

    for (const auto& it : active_) {
        if (nullptr != it.second) {
            bool success = it.second->Sign(
                preimage,
                role,
                signature,
                reason,
                opentxs::crypto::key::asymmetric::Role::Sign,
                hash);

            if (success) {
                haveSig = true;
                break;
            } else {
                LogError()(": Credential set ")(it.second->GetMasterCredID())(
                    " could not sign protobuf.")
                    .Flush();
            }
        }

        LogError()(": Did not find any credential sets capable of signing on "
                   "this nym.")
            .Flush();
    }

    return haveSig;
}

auto Nym::SocialMediaProfiles(const wot::claim::ClaimType type, bool active)
    const -> UnallocatedCString
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->SocialMediaProfiles(type, active);
}

auto Nym::SocialMediaProfileTypes() const
    -> const UnallocatedSet<wot::claim::ClaimType>
{
    eLock lock(shared_lock_);

    if (false == bool(contact_data_)) { init_claims(lock); }

    OT_ASSERT(contact_data_);

    return contact_data_->SocialMediaProfileTypes();
}

auto Nym::TransportKey(Data& pubkey, const opentxs::PasswordPrompt& reason)
    const -> OTSecret
{
    bool found{false};
    auto privateKey = api_.Factory().Secret(0);
    auto lock = sLock{shared_lock_};

    for (const auto& it : active_) {
        OT_ASSERT(nullptr != it.second);

        if (nullptr != it.second) {
            const identity::Authority& credSet = *it.second;
            found = credSet.TransportKey(pubkey, privateKey, reason);

            if (found) { break; }
        }
    }

    OT_ASSERT(found);

    return privateKey;
}

auto Nym::Unlock(
    const crypto::key::Asymmetric& dhKey,
    const std::uint32_t tag,
    const crypto::key::asymmetric::Algorithm type,
    const crypto::key::Symmetric& key,
    PasswordPrompt& reason) const noexcept -> bool
{
    for (const auto& [id, authority] : active_) {
        if (authority->Unlock(dhKey, tag, type, key, reason)) { return true; }
    }

    for (const auto& [id, authority] : revoked_sets_) {
        if (authority->Unlock(dhKey, tag, type, key, reason)) { return true; }
    }

    return false;
}

auto Nym::update_nym(
    const eLock& lock,
    const std::int32_t version,
    const opentxs::PasswordPrompt& reason) -> bool
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

auto Nym::Verify(const ProtobufType& input, proto::Signature& signature) const
    -> bool
{
    const auto copy{signature};
    signature.clear_signature();
    const auto plaintext = api_.Factory().InternalSession().Data(input);

    for (const auto& it : active_) {
        if (nullptr != it.second) {
            if (it.second->Verify(plaintext, copy)) { return true; }
        }
    }

    LogError()(OT_PRETTY_CLASS())("all ")(active_.size())(
        " authorities on nym ")(id_)(" failed to verify signature")
        .Flush();

    return false;
}

auto Nym::verify_pseudonym(const eLock& lock) const -> bool
{
    // If there are credentials, then we verify the Nym via his credentials.
    if (!active_.empty()) {
        // Verify Nym by his own credentials.
        for (const auto& it : active_) {
            const identity::Authority* pCredential = it.second.get();
            OT_ASSERT(nullptr != pCredential);

            // Verify all Credentials in the Authority, including source
            // verification for the master credential.
            if (!pCredential->VerifyInternally()) {
                LogConsole()(OT_PRETTY_CLASS())("Credential (")(
                    pCredential->GetMasterCredID())(") failed its "
                                                    "own internal "
                                                    "verification.")
                    .Flush();
                return false;
            }
        }
        return true;
    }
    LogError()(OT_PRETTY_CLASS())("No credentials.").Flush();
    return false;
}

auto Nym::VerifyPseudonym() const -> bool
{
    eLock lock(shared_lock_);

    return verify_pseudonym(lock);
}

auto Nym::WriteCredentials() const -> bool
{
    auto lock = sLock{shared_lock_};

    for (const auto& it : active_) {
        if (!it.second->WriteCredentials()) {
            LogError()(OT_PRETTY_CLASS())("Failed to save credentials.")
                .Flush();

            return false;
        }
    }

    return true;
}
}  // namespace opentxs::identity::implementation
