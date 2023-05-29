// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/wot/verification/Item.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

namespace proto
{
class Signature;
class Verification;
class VerificationGroup;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::internal
{
struct Item : virtual public verification::Item {
    using SerializedType = proto::Verification;

    auto Internal() const noexcept -> const internal::Item& final
    {
        return *this;
    }
    virtual auto Serialize(const api::Crypto& crypto) const noexcept
        -> SerializedType = 0;
    virtual auto Signature() const noexcept -> const proto::Signature& = 0;

    auto Internal() noexcept -> internal::Item& final { return *this; }

    ~Item() override = default;
};
}  // namespace opentxs::identity::wot::verification::internal
