// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "identity/credential/Verification.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Signature.pb.h>
#include <Verification.pb.h>
#include <VerificationGroup.pb.h>
#include <VerificationIdentity.pb.h>
#include <VerificationSet.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "identity/credential/Base.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/identity/Authority.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/key/asymmetric/Mode.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/credential/Verification.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::VerificationCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const crypto::Parameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
    -> identity::credential::internal::Verification*
{
    using ReturnType = identity::credential::implementation::Verification;

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

auto Factory::VerificationCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized)
    -> identity::credential::internal::Verification*
{
    using ReturnType = identity::credential::implementation::Verification;

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
auto Verification::SigningForm(const proto::Verification& item)
    -> proto::Verification
{
    proto::Verification signingForm(item);
    signingForm.clear_sig();

    return signingForm;
}

// static
auto Verification::VerificationID(
    const api::Session& api,
    const proto::Verification& item) -> UnallocatedCString
{
    return api.Factory()
        .InternalSession()
        .IdentifierFromPreimage(item)
        .asBase58(api.Crypto());
}
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential::implementation
{
Verification::Verification(
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
          identity::CredentialRole::Verify,
          crypto::key::asymmetric::Mode::Null,
          get_master_id(api, master))
    , data_(
          [&](const crypto::Parameters& params)
              -> const proto::VerificationSet {
              auto proto = proto::VerificationSet{};
              params.Internal().GetVerificationSet(proto);
              return proto;
          }(params))
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init(master, reason);
}

Verification::Verification(
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
    , data_(serialized.verification())
{
    Lock lock(lock_);
    init_serialized(lock);
}

auto Verification::GetVerificationSet(
    proto::VerificationSet& verificationSet) const -> bool
{
    verificationSet = proto::VerificationSet(data_);

    return true;
}

auto Verification::serialize(
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

    *serializedCredential->mutable_verification() = data_;

    return serializedCredential;
}

auto Verification::verify_internally(const Lock& lock) const -> bool
{
    // Perform common Credential verifications
    if (!Base::verify_internally(lock)) { return false; }

    for (const auto& nym : data_.internal().identity()) {
        for (const auto& claim : nym.verification()) {
            bool valid = parent_.Verify(claim);

            if (!valid) {
                LogError()(OT_PRETTY_CLASS())("Invalid claim verification.")
                    .Flush();

                return false;
            }
        }
    }

    return true;
}
}  // namespace opentxs::identity::credential::implementation
