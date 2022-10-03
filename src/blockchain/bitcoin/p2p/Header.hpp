// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <tuple>

#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
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

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::bitcoin
{
class Header final
{
public:
    struct BitcoinFormat {
        MagicField magic_;
        CommandField command_;
        PayloadSizeField length_;
        ChecksumField checksum_;

        auto Checksum() const noexcept -> ByteArray;
        auto Command() const noexcept -> bitcoin::Command;
        auto CheckNetwork(const blockchain::Type& chain) const noexcept -> bool;
        auto PayloadSize() const noexcept -> std::size_t;

        BitcoinFormat(const Data& in) noexcept(false);
        BitcoinFormat(const zmq::Frame& in) noexcept(false);
        BitcoinFormat(
            const blockchain::Type network,
            const bitcoin::Command command,
            const std::size_t payload,
            const ByteArray checksum) noexcept(false);

    private:
        BitcoinFormat(const void* data, const std::size_t size) noexcept(false);
    };

    static auto Size() noexcept -> std::size_t { return sizeof(BitcoinFormat); }

    auto Command() const noexcept -> bitcoin::Command { return command_; }
    auto Serialize(const AllocateOutput out) const noexcept -> bool;
    auto Network() const noexcept -> blockchain::Type { return chain_; }
    auto PayloadSize() const noexcept -> std::size_t { return payload_size_; }
    auto Checksum() const noexcept -> const opentxs::Data& { return checksum_; }

    auto SetChecksum(const std::size_t payload, ByteArray&& checksum) noexcept
        -> void;

    Header(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;
    Header(
        const api::Session& api,
        const blockchain::Type network,
        const bitcoin::Command command,
        const std::size_t payload,
        const ByteArray checksum) noexcept;
    Header() = delete;
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;

    ~Header() = default;

private:
    static constexpr auto header_size_ = 24_uz;

    blockchain::Type chain_;
    bitcoin::Command command_;
    std::size_t payload_size_;
    ByteArray checksum_;
};
}  // namespace opentxs::blockchain::p2p::bitcoin
