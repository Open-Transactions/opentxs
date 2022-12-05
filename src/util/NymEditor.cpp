// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::UnitType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType

#include "opentxs/util/NymEditor.hpp"  // IWYU pragma: associated

#include <ContactData.pb.h>
#include <Nym.pb.h>  // IWYU pragma: keep
#include <utility>

#include "internal/identity/Nym.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
NymData::NymData(
    const api::session::Factory& factory,
    std::mutex& objectMutex,
    const std::shared_ptr<identity::Nym>& nym,
    LockedSave save)
    : factory_{factory}
    , object_lock_{new Lock(objectMutex)}
    , locked_save_callback_{new LockedSave(save)}
    , nym_(nym)
{
    OT_ASSERT(object_lock_);
    OT_ASSERT(locked_save_callback_);
}

NymData::NymData(NymData&& rhs)
    : factory_{rhs.factory_}
    , object_lock_(std::move(rhs.object_lock_))
    , locked_save_callback_(std::move(rhs.locked_save_callback_))
    , nym_(std::move(rhs.nym_))
{
}

// This constructor is only used by Swig.  Swig doesn't support move
// constructors, so this copy constructor implements move semantics.
NymData::NymData(const NymData& rhs)
    : NymData(std::move(const_cast<NymData&>(rhs)))
{
}

auto NymData::AddChildKeyCredential(
    const identifier::Generic& strMasterID,
    const crypto::Parameters& nymParameters,
    const PasswordPrompt& reason) -> UnallocatedCString
{
    return nym().AddChildKeyCredential(strMasterID, nymParameters, reason);
}

auto NymData::AddClaim(const Claim& claim, const PasswordPrompt& reason) -> bool
{
    return nym().AddClaim(claim, reason);
}

auto NymData::DeleteClaim(
    const identifier::Generic& id,
    const PasswordPrompt& reason) -> bool
{
    return nym().DeleteClaim(id, reason);
}

auto NymData::AddContract(
    const UnallocatedCString& instrumentDefinitionID,
    const UnitType currency,
    const bool primary,
    const bool active,
    const PasswordPrompt& reason) -> bool
{
    auto id = factory_.UnitIDFromBase58(instrumentDefinitionID);

    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid instrument definition id.")
            .Flush();

        return false;
    }

    return nym().AddContract(id, currency, reason, primary, active);
}

auto NymData::AddEmail(
    const UnallocatedCString& value,
    const bool primary,
    const bool active,
    const PasswordPrompt& reason) -> bool
{
    return nym().AddEmail(value, reason, primary, active);
}

auto NymData::AddPaymentCode(
    const UnallocatedCString& code,
    const UnitType currency,
    const bool primary,
    const bool active,
    const PasswordPrompt& reason) -> bool
{
    auto paymentCode = factory_.PaymentCode(code);

    if (false == paymentCode.Valid()) {
        LogError()(OT_PRETTY_CLASS())("Invalid payment code.").Flush();

        return false;
    }

    return nym().AddPaymentCode(paymentCode, currency, reason, primary, active);
}

auto NymData::AddPhoneNumber(
    const UnallocatedCString& value,
    const bool primary,
    const bool active,
    const PasswordPrompt& reason) -> bool
{
    return nym().AddPhoneNumber(value, reason, primary, active);
}

auto NymData::AddPreferredOTServer(
    const UnallocatedCString& id,
    const bool primary,
    const PasswordPrompt& reason) -> bool
{
    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid server id.").Flush();

        return false;
    }

    return nym().AddPreferredOTServer(
        factory_.NotaryIDFromBase58(id), reason, primary);
}

auto NymData::AddSocialMediaProfile(
    const UnallocatedCString& value,
    const identity::wot::claim::ClaimType type,
    const bool primary,
    const bool active,
    const PasswordPrompt& reason) -> bool
{
    return nym().AddSocialMediaProfile(value, type, reason, primary, active);
}

auto NymData::asPublicNym() const -> proto::Nym
{
    auto serialized = proto::Nym{};
    if (false == nym().Internal().Serialize(serialized)) {}
    return serialized;
}

auto NymData::BestEmail() const -> UnallocatedCString
{
    return nym().BestEmail();
}

auto NymData::BestPhoneNumber() const -> UnallocatedCString
{
    return nym().BestPhoneNumber();
}

auto NymData::BestSocialMediaProfile(
    const identity::wot::claim::ClaimType type) const -> UnallocatedCString
{
    return nym().BestSocialMediaProfile(type);
}

auto NymData::Claims() const -> const identity::wot::claim::Data&
{
    return nym().Claims();
}

auto NymData::data() const -> const identity::wot::claim::Data&
{
    return nym().Claims();
}

auto NymData::EmailAddresses(bool active) const -> UnallocatedCString
{
    return nym().EmailAddresses(active);
}

auto NymData::HaveContract(
    const identifier::UnitDefinition& instrumentDefinitionID,
    const UnitType currency,
    const bool primary,
    const bool active) const -> bool
{
    OT_ASSERT(nym_);

    const auto contracts = nym().Contracts(currency, active);

    if (0 == contracts.size()) { return false; }

    const auto& data = nym().Claims();

    for (const auto& id : contracts) {
        const auto& claim = data.Claim(id);

        OT_ASSERT(claim);

        const auto value = factory_.UnitIDFromBase58(claim->Value());

        if (false == (instrumentDefinitionID == value)) { continue; }

        if ((false == primary) || claim->isPrimary()) { return true; }
    }

    return false;
}

auto NymData::Name() const -> UnallocatedCString { return nym().Name(); }

auto NymData::Nym() const -> const identity::Nym& { return nym(); }

auto NymData::nym() -> identity::Nym&
{
    OT_ASSERT(nym_);

    return *nym_;
}

auto NymData::nym() const -> const identity::Nym&
{
    OT_ASSERT(nym_);

    return *nym_;
}

auto NymData::PaymentCode(const UnitType currency) const -> UnallocatedCString
{
    return Contact::PaymentCode(data(), currency);
}

auto NymData::PhoneNumbers(bool active) const -> UnallocatedCString
{
    return nym().PhoneNumbers(active);
}

auto NymData::PreferredOTServer() const -> UnallocatedCString
{
    return data().PreferredOTServer().asBase58(Context().Crypto());
}

auto NymData::PrintContactData() const -> UnallocatedCString
{
    return identity::wot::claim::Data::PrintContactData([&] {
        auto proto = proto::ContactData{};
        data().Serialize(proto, true);

        return proto;
    }());
}

void NymData::Release() { release(); }

void NymData::release()
{
    if (locked_save_callback_) {
        auto callback = *locked_save_callback_;
        callback(this, *object_lock_);
    }

    locked_save_callback_.reset();

    if (object_lock_) {
        object_lock_->unlock();
        object_lock_.reset();
    }

    nym_.reset();
}

auto NymData::SetCommonName(
    const UnallocatedCString& name,
    const PasswordPrompt& reason) -> bool
{
    return nym().SetCommonName(name, reason);
}

auto NymData::SetContactData(
    const proto::ContactData& data,
    const PasswordPrompt& reason) -> bool
{
    return nym().Internal().SetContactData(data, reason);
}

auto NymData::SetContactData(
    const ReadView& bytes,
    const PasswordPrompt& reason) -> bool
{
    auto data = proto::Factory<proto::ContactData>(bytes);
    return SetContactData(data, reason);
}

auto NymData::SetScope(
    const identity::wot::claim::ClaimType type,
    const UnallocatedCString& name,
    const bool primary,
    const PasswordPrompt& reason) -> bool
{
    return nym().SetScope(type, name, reason, primary);
}

auto NymData::SocialMediaProfiles(
    const identity::wot::claim::ClaimType type,
    bool active) const -> UnallocatedCString
{
    return nym().SocialMediaProfiles(type, active);
}

auto NymData::SocialMediaProfileTypes() const
    -> UnallocatedSet<identity::wot::claim::ClaimType>
{
    return nym().SocialMediaProfileTypes();
}

auto NymData::Type() const -> identity::wot::claim::ClaimType
{
    return data().Type();
}

auto NymData::Valid() const -> bool { return bool(nym_); }

NymData::~NymData() { release(); }
}  // namespace opentxs
