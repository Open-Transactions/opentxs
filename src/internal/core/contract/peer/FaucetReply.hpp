// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/PeerReply.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
class Faucet;
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTFaucetReply = SharedPimpl<contract::peer::reply::Faucet>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class Faucet : virtual public peer::Reply
{
public:
    Faucet(const Faucet&) = delete;
    Faucet(Faucet&&) = delete;
    auto operator=(const Faucet&) -> Faucet& = delete;
    auto operator=(Faucet&&) -> Faucet& = delete;

    ~Faucet() override = default;

protected:
    Faucet() noexcept = default;

private:
    friend OTFaucetReply;

#ifndef _WIN32
    auto clone() const noexcept -> Faucet* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::reply