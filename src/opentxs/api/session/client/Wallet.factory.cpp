// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "opentxs/api/session/client/WalletPrivate.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto WalletAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::internal::Wallet>
{
    using ReturnType = api::session::client::WalletPrivate;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
