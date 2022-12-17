// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"

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
class Factory;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Filteradd final : virtual public internal::Filteradd,
                        public implementation::Message
{
public:
    auto Element() const noexcept -> ByteArray final { return element_; }

    Filteradd(
        const api::Session& api,
        const blockchain::Type network,
        const Data& element) noexcept;
    Filteradd(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const Data& element) noexcept;
    Filteradd(const Filteradd&) = delete;
    Filteradd(Filteradd&&) = delete;
    auto operator=(const Filteradd&) -> Filteradd& = delete;
    auto operator=(Filteradd&&) -> Filteradd& = delete;

    ~Filteradd() final = default;

private:
    const ByteArray element_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
