// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class ByteArray;
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Message : virtual public bitcoin::Message
{
public:
    auto Encode() const -> ByteArray final;
    auto header() const noexcept -> const Header& final { return *header_; }
    using bitcoin::Message::payload;
    auto payload() const noexcept -> ByteArray final;
    auto Transmit() const noexcept -> std::pair<zmq::Frame, zmq::Frame> final;

    auto header() noexcept -> Header& { return *header_; }

    Message() = delete;
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message&&) -> Message& = delete;

    ~Message() override = default;

protected:
    const ot::api::Session& api_;
    std::unique_ptr<Header> header_;

    static constexpr auto verify_hash_size(std::size_t size) noexcept -> bool
    {
        return size == standard_hash_size_;
    }

    auto verify_checksum() const noexcept(false) -> void;

    auto init_hash() noexcept -> void;

    Message(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;
    Message(const api::Session& api, std::unique_ptr<Header> header) noexcept;

private:
    auto calculate_checksum(const Data& payload) const noexcept -> ByteArray;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
