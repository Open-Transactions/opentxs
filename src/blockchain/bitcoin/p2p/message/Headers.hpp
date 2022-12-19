// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <span>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
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

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Headers final : virtual public internal::Headers,
                      public implementation::Message
{
public:
    auto get() const noexcept -> std::span<const value_type> final
    {
        return payload_;
    }

    auto get() noexcept -> std::span<value_type> final { return payload_; }

    Headers(
        const api::Session& api,
        const blockchain::Type network,
        UnallocatedVector<value_type>&& headers) noexcept;
    Headers(
        const api::Session& api,
        std::unique_ptr<Header> header,
        UnallocatedVector<value_type>&& headers) noexcept;
    Headers(const Headers&) = delete;
    Headers(Headers&&) = delete;
    auto operator=(const Headers&) -> Headers& = delete;
    auto operator=(Headers&&) -> Headers& = delete;

    ~Headers() final = default;

private:
    UnallocatedVector<value_type> payload_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
