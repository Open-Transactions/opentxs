// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/reply/Base.hpp"
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
namespace internal
{
class Bailment;
}  // namespace internal
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTBailmentReply = SharedPimpl<contract::peer::reply::internal::Bailment>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::internal
{
class Bailment : virtual public internal::Reply
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
};
}  // namespace opentxs::contract::peer::reply::internal
