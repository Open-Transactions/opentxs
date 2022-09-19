// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace otx
{
namespace blind
{
class Mint;
}  // namespace blind
}  // namespace otx
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::notary
{
using MintSeriesID = CString;
using MintSeries = Map<MintSeriesID, otx::blind::Mint>;

// WARNING update print function if new values are added or removed
enum class Job : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    queue_unitid = OT_ZMQ_INTERNAL_SIGNAL + 0,
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

auto print(Job) noexcept -> std::string_view;
}  // namespace opentxs::api::session::notary
