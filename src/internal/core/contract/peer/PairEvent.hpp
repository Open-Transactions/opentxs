// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/core/contract/peer/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class PairEvent;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::internal
{
struct PairEvent {
    std::uint32_t version_;
    PairEventType type_;
    UnallocatedCString issuer_;

    PairEvent(const ReadView);
    PairEvent() = delete;
    PairEvent(const PairEvent&) = delete;
    PairEvent(PairEvent&&) = delete;
    auto operator=(const PairEvent&) -> PairEvent& = delete;
    auto operator=(PairEvent&&) -> PairEvent& = delete;

private:
    PairEvent(
        const std::uint32_t,
        const PairEventType,
        const UnallocatedCString&);
    PairEvent(const proto::PairEvent&);
};
}  // namespace opentxs::contract::peer::internal
