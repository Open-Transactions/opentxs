// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/bitcoin/node/Manager.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>  // IWYU pragma: keep
#include <memory>

#include "internal/blockchain/node/Factory.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkBitcoin(
    const api::Session& api,
    const blockchain::Type type,
    const blockchain::node::internal::Config& config,
    std::string_view seednode,
    std::string_view syncEndpoint) noexcept
    -> std::shared_ptr<blockchain::node::Manager>
{
    using ReturnType = blockchain::node::base::Bitcoin;

    return std::make_shared<ReturnType>(
        api, type, config, seednode, syncEndpoint);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::base
{
Bitcoin::Bitcoin(
    const api::Session& api,
    const Type type,
    const internal::Config& config,
    std::string_view seednode,
    std::string_view syncEndpoint)
    : ot_super(api, type, config, seednode, syncEndpoint)
{
    init();
}

Bitcoin::~Bitcoin() { Shutdown(); }
}  // namespace opentxs::blockchain::node::base
