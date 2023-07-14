// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Verification.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Signature.pb.h>
#include <VerificationGroup.pb.h>
#include <VerificationIdentity.pb.h>
#include <VerificationItem.pb.h>
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
#include "opentxs/crypto/asymmetric/Mode.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/CredentialRole.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
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
auto Verification::SigningForm(const proto::VerificationItem& item)
    -> proto::VerificationItem
{
    proto::VerificationItem signingForm(item);
    signingForm.clear_sig();

    return signingForm;
}

auto Verification::VerificationID(
    const api::Session& api,
    const proto::VerificationItem& item) -> UnallocatedCString
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
          crypto::asymmetric::Mode::Null,
          master.ID())
    , data_(
          [&](const crypto::Parameters& parameters)
              -> const proto::VerificationSet {
              auto proto = proto::VerificationSet{};
              parameters.Internal().GetVerificationSet(proto);
              return proto;
          }(params))
{
    first_time_init(set_name_from_id_);
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
    init_serialized();
}

auto Verification::GetVerificationSet(
    proto::VerificationSet& verificationSet) const -> bool
{
    verificationSet = proto::VerificationSet(data_);

    return true;
}

auto Verification::id_form() const -> std::shared_ptr<SerializedType>
{
    auto out = Base::id_form();
    out->set_mode(translate(crypto::asymmetric::Mode::Null));
    *out->mutable_verification() = data_;

    return out;
}

auto Verification::serialize(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto out = Base::serialize(asPrivate, asSigned);

    if (asSigned) {
        if (auto sig = MasterSignature(); sig) {
            *out->add_signature() = *sig;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to get master signature.")
                .Flush();
        }
    }

    return out;
}

auto Verification::verify_internally() const -> bool
{
    // Perform common Credential verifications
    if (!Base::verify_internally()) { return false; }

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
