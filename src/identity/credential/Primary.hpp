// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::CredentialRole
// IWYU pragma: no_forward_declare opentxs::identity::NymCapability
// IWYU pragma: no_forward_declare opentxs::identity::SourceProofType
// IWYU pragma: no_forward_declare opentxs::identity::SourceType
// IWYU pragma: no_forward_declare opentxs::proto::SourceProofType

#pragma once

#include <Enums.pb.h>
#include <SourceProof.pb.h>
#include <robin_hood.h>
#include <memory>

#include "identity/credential/Base.hpp"
#include "identity/credential/Key.hpp"
#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/SourceProofType.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
namespace internal
{
class Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace proto
{
class Credential;
class HDPath;
class Signature;
}  // namespace proto

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Primary final : virtual public credential::internal::Primary,
                      credential::implementation::Key
{
public:
    auto hasCapability(const NymCapability& capability) const -> bool final;
    auto Path(proto::HDPath& output) const -> bool final;
    auto Path() const -> UnallocatedCString final;
    using Base::Verify;
    auto Verify(
        const proto::Credential& credential,
        const identity::CredentialRole& role,
        const identifier::Generic& masterID,
        const proto::Signature& masterSig) const -> bool final;

    Primary() = delete;
    Primary(const Primary&) = delete;
    Primary(Primary&&) = delete;
    auto operator=(const Primary&) -> Primary& = delete;
    auto operator=(Primary&&) -> Primary& = delete;

    ~Primary() final = default;

private:
    friend opentxs::Factory;

    using SourceProofTypeMap = robin_hood::
        unordered_flat_map<identity::SourceProofType, proto::SourceProofType>;
    using SourceProofTypeReverseMap = robin_hood::
        unordered_flat_map<proto::SourceProofType, identity::SourceProofType>;

    static const VersionConversionMap credential_to_master_params_;

    const proto::SourceProof source_proof_;

    static auto source_proof(const crypto::Parameters& params)
        -> proto::SourceProof;
    static auto sourceprooftype_map() noexcept -> const SourceProofTypeMap&;
    static auto translate(const identity::SourceProofType in) noexcept
        -> proto::SourceProofType;
    static auto translate(const proto::SourceProofType in) noexcept
        -> identity::SourceProofType;

    auto serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<internal::Base::SerializedType> final;
    auto verify_against_source(const Lock& lock) const -> bool;
    auto verify_internally(const Lock& lock) const -> bool final;

    void sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false) final;

    Primary(
        const api::Session& api,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Primary(
        const api::Session& api,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& serializedCred) noexcept(false);
};
}  // namespace opentxs::identity::credential::implementation
