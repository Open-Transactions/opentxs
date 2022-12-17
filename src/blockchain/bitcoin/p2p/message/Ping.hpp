// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/Types.hpp"

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
class Ping final : public internal::Ping, public implementation::Message
{
public:
    struct BitcoinFormat_60001 {
        NonceField nonce_{};

        BitcoinFormat_60001(const bitcoin::Nonce nonce) noexcept;
        BitcoinFormat_60001() noexcept;
    };

    auto Nonce() const noexcept -> bitcoin::Nonce final { return nonce_; }

    Ping(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Nonce nonce) noexcept;
    Ping(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const bitcoin::Nonce nonce) noexcept;
    Ping(const Ping&) = delete;
    Ping(Ping&&) = delete;
    auto operator=(const Ping&) -> Ping& = delete;
    auto operator=(Ping&&) -> Ping& = delete;

    ~Ping() final = default;

private:
    const bitcoin::Nonce nonce_{};

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
