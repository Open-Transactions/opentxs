// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace blockchain
{
namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto

using OTBlockchainAddress = Pimpl<blockchain::p2p::Address>;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace p2p
{
class OPENTXS_EXPORT Address
{
public:
    using SerializedType = proto::BlockchainPeerAddress;

    virtual auto Bytes() const noexcept -> OTData = 0;
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    virtual auto Display() const noexcept -> std::string = 0;
    virtual auto ID() const noexcept -> const Identifier& = 0;
    virtual auto LastConnected() const noexcept -> Time = 0;
    virtual auto Port() const noexcept -> std::uint16_t = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(SerializedType& out) const noexcept
        -> bool = 0;
    virtual auto Services() const noexcept -> std::pmr::set<Service> = 0;
    virtual auto Style() const noexcept -> Protocol = 0;
    virtual auto Type() const noexcept -> Network = 0;

    virtual void AddService(const Service service) noexcept = 0;
    virtual void RemoveService(const Service service) noexcept = 0;
    virtual void SetLastConnected(const Time& time) noexcept = 0;
    virtual void SetServices(
        const std::pmr::set<Service>& services) noexcept = 0;

    virtual ~Address() = default;

protected:
    Address() noexcept = default;

private:
    friend OTBlockchainAddress;

    virtual auto clone() const noexcept -> Address* = 0;

    Address(const Address&) = delete;
    Address(Address&&) = delete;
    auto operator=(const Address&) -> Address& = delete;
    auto operator=(Address&&) -> Address& = delete;
};
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
