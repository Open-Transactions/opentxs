// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/ClientPrivate.hpp"  // IWYU pragma: associated

#include <ClientContext.pb.h>
#include <Context.pb.h>

namespace opentxs::otx::context
{
ClientPrivate::ClientPrivate(const VersionNumber targetVersion) noexcept
    : ConsensusPrivate(targetVersion)
    , open_cron_items_()
{
}

ClientPrivate::ClientPrivate(
    const api::Session& api,
    const VersionNumber targetVersion,
    const proto::Context& serialized) noexcept
    : ConsensusPrivate(api, targetVersion, serialized)
    , open_cron_items_([&] {
        auto out = decltype(open_cron_items_){};

        if (serialized.has_clientcontext()) {
            for (const auto& it : serialized.clientcontext().opencronitems()) {
                out.insert(it);
            }
        }

        return out;
    }())
{
}
}  // namespace opentxs::otx::context
