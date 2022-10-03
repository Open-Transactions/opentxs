// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "opentxs/Export.hpp"

namespace opentxs::blockchain::p2p
{
class OPENTXS_EXPORT Peer
{
public:
    using ConnectionStatus = std::shared_future<bool>;
    using Handshake = std::shared_future<void>;
    using Verify = std::shared_future<void>;
    using Subscribe = std::shared_future<void>;

    virtual auto Connected() const noexcept -> ConnectionStatus = 0;
    virtual auto HandshakeComplete() const noexcept -> Handshake = 0;

    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;

    virtual ~Peer() = default;

protected:
    Peer() noexcept = default;
};
}  // namespace opentxs::blockchain::p2p
