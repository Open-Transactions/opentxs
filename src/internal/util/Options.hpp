// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"

namespace opentxs::internal
{
class Options
{
public:
    struct Listener {
        network::blockchain::Transport external_type_{};
        ByteArray external_address_{};
        network::blockchain::Transport local_type_{};
        ByteArray local_address_{};
    };

    virtual auto OTDHTListeners() const noexcept
        -> std::span<const Listener> = 0;

    Options() noexcept = default;
    Options(const Options&) noexcept = default;

    virtual ~Options() = default;
};
}  // namespace opentxs::internal
