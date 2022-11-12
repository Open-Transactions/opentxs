// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::internal
{
struct Config {
    BlockchainProfile profile_{BlockchainProfile::desktop};
    bool provide_sync_server_{false};
    bool disable_wallet_{false};

    auto PeerTarget(blockchain::Type) const noexcept -> std::size_t;
    auto Print(alloc::Default alloc = {}) const noexcept -> CString;
};
}  // namespace opentxs::blockchain::node::internal
