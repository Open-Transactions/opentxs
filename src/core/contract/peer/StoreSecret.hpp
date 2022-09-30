// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <PeerEnums.pb.h>

#include "Proto.hpp"
#include "core/contract/peer/PeerRequest.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class Nym;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request::implementation
{
class StoreSecret final : public request::StoreSecret,
                          public peer::implementation::Request
{
public:
    auto asStoreSecret() const noexcept -> const request::StoreSecret& final
    {
        return *this;
    }

    StoreSecret(
        const api::Session& api,
        const Nym_p& nym,
        const identifier::Nym& recipientID,
        const SecretType type,
        const UnallocatedCString& primary,
        const UnallocatedCString& secondary,
        const identifier::Notary& serverID);
    StoreSecret(
        const api::Session& api,
        const Nym_p& nym,
        const proto::PeerRequest& serialized);
    StoreSecret() = delete;
    StoreSecret(const StoreSecret&);
    StoreSecret(StoreSecret&&) = delete;
    auto operator=(const StoreSecret&) -> StoreSecret& = delete;
    auto operator=(StoreSecret&&) -> StoreSecret& = delete;

    ~StoreSecret() final = default;

private:
    friend opentxs::Factory;

    static constexpr auto current_version_ = VersionNumber{4};

    const SecretType secret_type_;
    const UnallocatedCString primary_;
    const UnallocatedCString secondary_;

    auto clone() const noexcept -> StoreSecret* final
    {
        return new StoreSecret(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;
};
}  // namespace opentxs::contract::peer::request::implementation
