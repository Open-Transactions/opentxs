// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/contract/peer/Request.hpp"  // IWYU pragma: associated

namespace opentxs::contract::peer::internal
{
auto Request::Serialize(protobuf::PeerRequest&) const noexcept -> bool
{
    return false;
}

auto Request::SetReceived(Time) noexcept -> void {}
}  // namespace opentxs::contract::peer::internal
