// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <utility>

#include "opentxs/api/session/ClientPrivate.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::factory
{
auto ClientSession(
    const api::Context& parent,
    Flag& running,
    Options&& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::filesystem::path& dataFolder,
    const int instance) noexcept
    -> std::shared_ptr<api::session::internal::Client>
{
    using ReturnType = api::session::ClientPrivate;

    try {
        return std::make_shared<ReturnType>(
            parent,
            running,
            std::move(args),
            config,
            crypto,
            context,
            dataFolder,
            instance);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
