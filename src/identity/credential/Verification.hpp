// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/VerificationSet.pb.h>
#include <memory>

#include "identity/credential/Base.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "opentxs/identity/Types.internal.hpp"
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

namespace identity
{
namespace internal
{
class Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace protobuf
{
class Credential;
}  // namespace protobuf

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Verification final : virtual public credential::internal::Verification,
                           public credential::implementation::Base
{
public:
    auto GetVerificationSet(protobuf::VerificationSet& verificationSet) const
        -> bool final;

    Verification() = delete;
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;

    ~Verification() final = default;

private:
    friend opentxs::Factory;

    const protobuf::VerificationSet data_;

    auto id_form() const -> std::shared_ptr<SerializedType> final;
    auto serialize(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<Base::SerializedType> final;
    auto verify_internally() const -> bool final;

    Verification(
        const api::Session& api,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const internal::Primary& master,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Verification(
        const api::Session& api,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const internal::Primary& master,
        const protobuf::Credential& credential) noexcept(false);
};
}  // namespace opentxs::identity::credential::implementation
