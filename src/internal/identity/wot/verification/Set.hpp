// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/identity/wot/verification/Item.hpp"
#include "opentxs/identity/wot/verification/Set.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace protobuf
{
class Signature;
class VerificationGroup;
class VerificationItem;
class VerificationSet;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::internal
{
struct Set : virtual public verification::Set {
    using SerializedType = protobuf::VerificationSet;

    virtual operator SerializedType() const noexcept = 0;

    virtual auto API() const noexcept -> const api::Session& = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    auto Private() const noexcept -> const internal::Set& final
    {
        return *this;
    }

    using verification::Set::AddItem;
    virtual auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool = 0;
    auto Private() noexcept -> internal::Set& final { return *this; }
    virtual void Register(
        const identifier::Generic& id,
        const bool external) noexcept = 0;
    virtual void Unregister(const identifier::Generic& id) noexcept = 0;
    virtual auto UpgradeGroupVersion(const VersionNumber groupVersion) noexcept
        -> bool = 0;

    ~Set() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
