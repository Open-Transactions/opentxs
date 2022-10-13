// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/PeerRequest.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class StoreSecret;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTStoreSecret = SharedPimpl<contract::peer::request::StoreSecret>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class StoreSecret : virtual public peer::Request
{
public:
    StoreSecret(const StoreSecret&) = delete;
    StoreSecret(StoreSecret&&) = delete;
    auto operator=(const StoreSecret&) -> StoreSecret& = delete;
    auto operator=(StoreSecret&&) -> StoreSecret& = delete;

    ~StoreSecret() override = default;

protected:
    StoreSecret() noexcept = default;

private:
    friend OTStoreSecret;

#ifndef _WIN32
    auto clone() const noexcept -> StoreSecret* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::request
