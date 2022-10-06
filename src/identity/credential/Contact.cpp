// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "identity/credential/Contact.hpp"  // IWYU pragma: associated

#include <Claim.pb.h>
#include <ContactData.pb.h>
#include <ContactItem.pb.h>
#include <Credential.pb.h>
#include <Signature.pb.h>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "identity/credential/Base.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/key/asymmetric/Mode.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
auto Factory::ContactCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const crypto::Parameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
    -> identity::credential::internal::Contact*
{
    using ReturnType = identity::credential::implementation::Contact;

    try {

        return new ReturnType(
            api, parent, source, master, parameters, version, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to create credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::ContactCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized)
    -> identity::credential::internal::Contact*
{
    using ReturnType = identity::credential::implementation::Contact;

    try {

        return new ReturnType(api, parent, source, master, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to deserialize credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::credential
{
// static
auto Contact::ClaimID(
    const api::Session& api,
    const UnallocatedCString& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item) -> UnallocatedCString
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(section);
    preimage.set_type(item.type());
    preimage.set_start(item.start());
    preimage.set_end(item.end());
    preimage.set_value(item.value());
    preimage.set_subtype(item.subtype());

    return String::Factory(ClaimID(api, preimage))->Get();
}

// static
auto Contact::ClaimID(
    const api::Session& api,
    const UnallocatedCString& nymid,
    const wot::claim::SectionType section,
    const wot::claim::ClaimType type,
    const Time start,
    const Time end,
    const UnallocatedCString& value,
    const UnallocatedCString& subtype) -> UnallocatedCString
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(translate(section));
    preimage.set_type(translate(type));
    preimage.set_start(Clock::to_time_t(start));
    preimage.set_end(Clock::to_time_t(end));
    preimage.set_value(value);
    preimage.set_subtype(subtype);

    return ClaimID(api, preimage).asBase58(api.Crypto());
}

// static
auto Contact::ClaimID(const api::Session& api, const proto::Claim& preimage)
    -> identifier::Generic
{
    return api.Factory().InternalSession().IdentifierFromPreimage(preimage);
}

// static
auto Contact::asClaim(
    const api::Session& api,
    const String& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item) -> Claim
{
    UnallocatedSet<std::uint32_t> attributes;

    for (const auto& attrib : item.attribute()) { attributes.insert(attrib); }

    return Claim{
        ClaimID(api, nymid.Get(), section, item),
        section,
        item.type(),
        item.value(),
        convert_stime(item.start()),
        convert_stime(item.end()),
        attributes};
}
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential::implementation
{
Contact::Contact(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const internal::Primary& master,
    const crypto::Parameters& params,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : credential::implementation::Base(
          api,
          parent,
          source,
          params,
          version,
          identity::CredentialRole::Contact,
          crypto::key::asymmetric::Mode::Null,
          get_master_id(api, master))
    , data_([&](const crypto::Parameters& params) -> const proto::ContactData {
        auto proto = proto::ContactData{};
        params.Internal().GetContactData(proto);
        return proto;
    }(params))
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init(master, reason);
}

Contact::Contact(
    const api::Session& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const internal::Primary& master,
    const proto::Credential& serialized) noexcept(false)
    : credential::implementation::Base(
          api,
          parent,
          source,
          serialized,
          get_master_id(api, serialized, master))
    , data_(serialized.contactdata())
{
    Lock lock(lock_);
    init_serialized(lock);
}

auto Contact::GetContactData(proto::ContactData& contactData) const -> bool
{
    contactData = proto::ContactData(data_);

    return true;
}

auto Contact::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto serializedCredential = Base::serialize(lock, asPrivate, asSigned);
    serializedCredential->set_mode(
        translate(crypto::key::asymmetric::Mode::Null));
    serializedCredential->clear_signature();  // this fixes a bug, but shouldn't

    if (asSigned) {
        auto masterSignature = MasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature =
                serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to get master signature.")
                .Flush();
        }
    }

    *serializedCredential->mutable_contactdata() = data_;

    return serializedCredential;
}

}  // namespace opentxs::identity::credential::implementation
