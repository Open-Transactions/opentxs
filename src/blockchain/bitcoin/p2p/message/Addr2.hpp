// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <utility>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
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

class Address;
}  // namespace p2p
}  // namespace blockchain

class ByteArray;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Addr2 final : public internal::Addr2, public implementation::Message
{
public:
    using AddressVector = Vector<blockchain::p2p::Address>;

    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return {this, 0}; }
    auto end() const noexcept -> const_iterator final
    {
        return {this, payload_.size()};
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }

    Addr2(
        const api::Session& api,
        const blockchain::Type network,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr2(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const ProtocolVersion version,
        AddressVector&& addresses) noexcept;
    Addr2(const Addr2&) = delete;
    Addr2(Addr2&&) = delete;
    auto operator=(const Addr2&) -> Addr2& = delete;
    auto operator=(Addr2&&) -> Addr2& = delete;

    ~Addr2() final = default;

private:
    const ProtocolVersion version_;
    const AddressVector payload_;

    using implementation::Message::payload;
    auto payload(Writer&& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
