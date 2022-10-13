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
class Bailment;
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTBailmentReply = SharedPimpl<contract::peer::reply::Bailment>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class Bailment : virtual public peer::Reply
{
public:
    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;

    ~Bailment() override = default;

protected:
    Bailment() noexcept = default;

private:
    friend OTBailmentReply;

#ifndef _WIN32
    auto clone() const noexcept -> Bailment* override = 0;
#endif
};
}  // namespace opentxs::contract::peer::reply
