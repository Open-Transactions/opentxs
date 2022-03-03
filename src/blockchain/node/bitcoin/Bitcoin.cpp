// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include "Proto.tpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"  // IWYU pragma: keep

namespace opentxs::factory
{
auto BlockchainNetworkBitcoin(
    const api::Session& api,
    const blockchain::Type type,
    const blockchain::node::internal::Config& config,
    const UnallocatedCString& seednode,
    const UnallocatedCString& syncEndpoint) noexcept
    -> std::unique_ptr<blockchain::node::internal::Network>
{
    using ReturnType = blockchain::node::base::Bitcoin;

    return std::make_unique<ReturnType>(
        api, type, config, seednode, syncEndpoint);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::base
{
Bitcoin::Bitcoin(
    const api::Session& api,
    const Type type,
    const internal::Config& config,
    const UnallocatedCString& seednode,
    const UnallocatedCString& syncEndpoint)
    : ot_super(api, type, config, seednode, syncEndpoint)
{
    init();
}

auto Bitcoin::instantiate_header(const ReadView payload) const noexcept
    -> std::unique_ptr<block::Header>
{
    using Type = block::Header::SerializedType;

    return std::unique_ptr<block::Header>{
        factory::BitcoinBlockHeader(api_, proto::Factory<Type>(payload))};
}

Bitcoin::~Bitcoin() { Shutdown(); }
}  // namespace opentxs::blockchain::node::base
