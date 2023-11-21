// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/ContextPrivate.hpp"

namespace opentxs::factory
{
auto Context(
    const network::zeromq::Context& zmq,
    const api::network::Asio& asio,
    const internal::ShutdownSender& sender,
    const Options& args,
    PasswordCaller* externalPasswordCallback) noexcept
    -> std::shared_ptr<api::internal::Context>
{
    using ReturnType = api::ContextPrivate;

    return std::make_shared<ReturnType>(
        args, zmq, asio, sender, externalPasswordCallback);
}
}  // namespace opentxs::factory
