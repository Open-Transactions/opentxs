// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Cmpctblock final : public implementation::Message
{
public:
    Cmpctblock(
        const api::Session& api,
        const blockchain::Type network,
        const Data& raw_cmpctblock) noexcept;
    Cmpctblock(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const Data& raw_cmpctblock) noexcept(false);
    Cmpctblock(const Cmpctblock&) = delete;
    Cmpctblock(Cmpctblock&&) = delete;
    auto operator=(const Cmpctblock&) -> Cmpctblock& = delete;
    auto operator=(Cmpctblock&&) -> Cmpctblock& = delete;

    ~Cmpctblock() final = default;

private:
    const ByteArray raw_cmpctblock_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
