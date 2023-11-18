// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/notary/FactoryPrivate.hpp"  // IWYU pragma: associated

#include "internal/otx/common/cron/OTCron.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Notary.internal.hpp"

namespace opentxs::api::session::notary
{
FactoryPrivate::FactoryPrivate(
    const internal::Notary& api,
    const api::Factory& parent)
    : session::FactoryPrivate(api, parent)
    , server_(api)
{
}

auto FactoryPrivate::Cron() const -> std::unique_ptr<OTCron>
{
    auto output = std::unique_ptr<opentxs::OTCron>{};
    output.reset(new opentxs::OTCron(server_.asNotaryPublic()));

    return output;
}
}  // namespace opentxs::api::session::notary
