// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class PeerRequest;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::internal
{
class Request
{
public:
    using serialized_type = protobuf::PeerRequest;

    [[nodiscard]] virtual auto Serialize(serialized_type& out) const noexcept
        -> bool;

    auto SetReceived(Time time) noexcept -> void;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request& rhs) noexcept -> Request& = delete;
    auto operator=(Request&& rhs) noexcept -> Request& = delete;

    virtual ~Request() = default;

protected:
    Request() = default;
};
}  // namespace opentxs::contract::peer::internal
