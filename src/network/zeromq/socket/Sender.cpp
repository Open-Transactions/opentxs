// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Sender.hpp"

//#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Sender::"

namespace opentxs::network::zeromq::socket::implementation
{
Sender::Sender(
    const zeromq::Context& context,
    const SocketType type,
    const zeromq::Socket::Direction direction)
    : Socket(context, type, direction)
{
}

bool Sender::deliver(zeromq::Message& message) const
{
    Lock lock(lock_);

    if (false == running_.get()) { return false; }

    return send_message(lock, message);
}
}  // namespace opentxs::network::zeromq::socket::implementation
