// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Factory;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const blockchain::block::TransactionHash& txid) noexcept
    -> identifier::Generic;
}  // namespace opentxs

namespace opentxs::blockchain::crypto
{
struct Notifications {
    Set<opentxs::PaymentCode> incoming_{};
    Set<opentxs::PaymentCode> outgoing_{};
    Set<opentxs::PaymentCode> neither_{};
};

using NotificationStatus = Map<blockchain::Type, Notifications>;

auto deserialize(std::span<const network::zeromq::Frame> in) noexcept -> Target;
auto serialize(const Target& target, Data& out) noexcept -> void;
auto serialize(const Target& target, network::zeromq::Message& out) noexcept
    -> void;
}  // namespace opentxs::blockchain::crypto
