// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/Context.internal.hpp"  // IWYU pragma: associated

#include <atomic>

#include "opentxs/api/ContextPrivate.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::api::internal
{
auto Context::MaxJobs() noexcept -> unsigned int
{
    return ContextPrivate::JobCount().load();
}

auto Context::SetMaxJobs(const opentxs::Options& args) noexcept -> void
{
    ContextPrivate::JobCount().store(args.MaxJobs());
}
}  // namespace opentxs::api::internal
