// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "api/session/notary/Factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "internal/api/session/Factory.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SessionFactoryAPI(const api::session::Notary& parent) noexcept
    -> std::unique_ptr<api::session::Factory>
{
    using ReturnType = api::session::notary::Factory;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::notary
{
Factory::Factory(const api::session::Notary& parent)
    : session::imp::Factory(parent)
    , server_(parent)
{
}

auto Factory::Cron() const -> std::unique_ptr<OTCron>
{
    auto output = std::unique_ptr<opentxs::OTCron>{};
    output.reset(new opentxs::OTCron(server_));

    return output;
}
}  // namespace opentxs::api::session::notary
