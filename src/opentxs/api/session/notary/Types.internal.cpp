// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/notary/Types.internal.hpp"  // IWYU pragma: associated

#include <string_view>

#include "opentxs/util/Container.hpp"

namespace opentxs::api::session::notary
{
auto print(Job value) noexcept -> std::string_view
{
    using namespace std::literals;
    static const auto map = Map<Job, std::string_view>{
        {Job::shutdown, "shutdown"sv},
        {Job::queue_unitid, "queue_unitid"sv},
        {Job::init, "init"sv},
        {Job::statemachine, "statemachine"sv},
    };

    try {
        return map.at(value);
    } catch (...) {

        return "Unknown Job type"sv;
    }
}
}  // namespace opentxs::api::session::notary
