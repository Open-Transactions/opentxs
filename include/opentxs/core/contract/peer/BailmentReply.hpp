// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class OPENTXS_EXPORT Bailment : virtual public peer::Reply
{
public:
    ~Bailment() override = default;

protected:
    Bailment() noexcept = default;

private:
    friend OTBailmentReply;

#ifndef _WIN32
    auto clone() const noexcept -> Bailment* override = 0;
#endif

    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;
};
}  // namespace opentxs::contract::peer::reply
