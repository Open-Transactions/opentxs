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
class Outbailment;
}  // namespace internal
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTOutbailmentReply =
    SharedPimpl<contract::peer::reply::internal::Outbailment>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply::internal
{
class Outbailment : virtual public internal::Reply
{
public:
    Outbailment(const Outbailment&) = delete;
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;

    ~Outbailment() override = default;

protected:
    Outbailment() noexcept = default;

private:
    friend OTOutbailmentReply;
};
}  // namespace opentxs::contract::peer::reply::internal
