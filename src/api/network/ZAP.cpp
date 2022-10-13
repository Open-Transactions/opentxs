// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "api/network/ZAP.hpp"  // IWYU pragma: associated

#include "2_Factory.hpp"
#include "internal/network/zeromq/zap/Callback.hpp"
#include "internal/network/zeromq/zap/Handler.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto Factory::ZAP(const network::zeromq::Context& context) -> api::network::ZAP*
{
    return new api::network::imp::ZAP(context);
}
}  // namespace opentxs

namespace opentxs::api::network::imp
{
ZAP::ZAP(const opentxs::network::zeromq::Context& context)
    : context_(context)
    , callback_(opentxs::network::zeromq::zap::Callback::Factory())
    , zap_(opentxs::network::zeromq::zap::Handler::Factory(
          context_,
          callback_,
          "ZAP"))
{
}

auto ZAP::RegisterDomain(
    const std::string_view domain,
    const opentxs::network::zeromq::zap::ReceiveCallback& callback) const
    -> bool
{
    return callback_->SetDomain(UnallocatedCString{domain}, callback);
}

auto ZAP::SetDefaultPolicy(
    const opentxs::network::zeromq::zap::Policy policy) const -> bool
{
    return callback_->SetPolicy(policy);
}
}  // namespace opentxs::api::network::imp
