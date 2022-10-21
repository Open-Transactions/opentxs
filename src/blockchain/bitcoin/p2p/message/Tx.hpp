// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

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

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Tx final : public internal::Tx, public implementation::Message
{
public:
    auto Transaction() const noexcept
        -> std::unique_ptr<const blockchain::bitcoin::block::Transaction> final;

    Tx(const api::Session& api,
       const blockchain::Type network,
       const ReadView transaction) noexcept;
    Tx(const api::Session& api,
       std::unique_ptr<Header> header,
       const ReadView transaction) noexcept(false);
    Tx(const Tx&) = delete;
    Tx(Tx&&) = delete;
    auto operator=(const Tx&) -> Tx& = delete;
    auto operator=(Tx&&) -> Tx& = delete;

    ~Tx() final = default;

private:
    const ByteArray payload_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
