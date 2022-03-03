// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "Proto.hpp"
#include "identity/credential/Base.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/VerificationSet.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
struct Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace proto
{
class Credential;
}  // namespace proto

class Factory;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Verification final : virtual public credential::internal::Verification,
                           public credential::implementation::Base
{
public:
    auto GetVerificationSet(proto::VerificationSet& verificationSet) const
        -> bool final;

    ~Verification() final = default;

private:
    friend opentxs::Factory;

    const proto::VerificationSet data_;

    auto serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<Base::SerializedType> final;
    auto verify_internally(const Lock& lock) const -> bool final;

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
        const proto::Credential& credential) noexcept(false);
    Verification() = delete;
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;
};
}  // namespace opentxs::identity::credential::implementation
