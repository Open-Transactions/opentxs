// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Notification : virtual public crypto::Notification,
                      virtual public Subaccount {
    auto InternalNotification() const noexcept
        -> const internal::Notification& final
    {
        return *this;
    }
    virtual auto Path() const noexcept -> proto::HDPath = 0;

    auto InternalNotification() noexcept -> internal::Notification& final
    {
        return *this;
    }
};
}  // namespace opentxs::blockchain::crypto::internal
