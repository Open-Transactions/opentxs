// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/ZeroMQPrivate.hpp"  // IWYU pragma: associated

namespace opentxs::api::network
{
ZeroMQPrivate::ZeroMQPrivate(
    const opentxs::network::zeromq::Context& context,
    const network::ZAP& zap) noexcept
    : context_(context)
    , zap_(zap)
{
}

ZeroMQPrivate::~ZeroMQPrivate() = default;
}  // namespace opentxs::api::network
