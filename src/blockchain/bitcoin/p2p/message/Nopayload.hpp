// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

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

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
template <typename InterfaceType>
class Nopayload final : virtual public InterfaceType,
                        public implementation::Message
{
public:
    Nopayload(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept
        : Message(api, network, command)
    {
        Message::init_hash();
    }
    Nopayload(const api::Session& api, std::unique_ptr<Header> header) noexcept
        : Message(api, std::move(header))
    {
    }
    Nopayload(const Nopayload&) = delete;
    Nopayload(Nopayload&&) = delete;
    auto operator=(const Nopayload&) -> Nopayload& = delete;
    auto operator=(Nopayload&&) -> Nopayload& = delete;

    ~Nopayload() final = default;

private:
    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final
    {
        try {
            auto buf = out.Reserve(0_uz);

            return 0_uz == buf.size();
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
