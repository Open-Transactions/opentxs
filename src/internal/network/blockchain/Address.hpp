// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class BlockchainPeerAddress;
}  // namespace protobuf

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::internal
{
class Address
{
public:
    virtual auto Bytes() const noexcept -> ByteArray;
    virtual auto Chain() const noexcept -> opentxs::blockchain::Type;
    virtual auto clone() const noexcept -> std::unique_ptr<Address>
    {
        return std::make_unique<Address>();
    }
    virtual auto Cookie() const noexcept -> ReadView;
    virtual auto Display() const noexcept -> UnallocatedCString;
    virtual auto ID() const noexcept -> const AddressID&;
    virtual auto Incoming() const noexcept -> bool;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Key() const noexcept -> ReadView;
    virtual auto LastConnected() const noexcept -> Time;
    virtual auto Port() const noexcept -> std::uint16_t;
    virtual auto PreviousLastConnected() const noexcept -> Time;
    virtual auto PreviousServices() const noexcept -> Set<bitcoin::Service>;
    virtual auto Serialize(protobuf::BlockchainPeerAddress& out) const noexcept
        -> bool;
    virtual auto Services() const noexcept -> Set<bitcoin::Service>;
    virtual auto Style() const noexcept -> Protocol;
    virtual auto Subtype() const noexcept -> Transport;
    virtual auto Type() const noexcept -> Transport;

    virtual auto AddService(const bitcoin::Service service) noexcept -> void;
    virtual auto RemoveService(const bitcoin::Service service) noexcept -> void;
    virtual auto SetIncoming(bool value) noexcept -> void;
    virtual auto SetLastConnected(const Time& time) noexcept -> void;
    virtual auto SetServices(const Set<bitcoin::Service>& services) noexcept
        -> void;

    virtual ~Address() = default;
};
}  // namespace opentxs::network::blockchain::internal
