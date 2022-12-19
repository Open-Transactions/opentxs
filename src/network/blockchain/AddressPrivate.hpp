// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/network/blockchain/Address.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class AddressPrivate : public internal::Address
{
public:
    virtual auto Bytes() const noexcept -> ByteArray { return {}; }
    virtual auto Chain() const noexcept -> opentxs::blockchain::Type
    {
        return {};
    }
    virtual auto clone() const noexcept -> std::unique_ptr<AddressPrivate>
    {
        return std::make_unique<AddressPrivate>();
    }
    virtual auto Display() const noexcept -> UnallocatedCString { return {}; }
    virtual auto ID() const noexcept -> const identifier::Generic&
    {
        static const auto blank = identifier::Generic{};

        return blank;
    }
    auto Incoming() const noexcept -> bool override { return {}; }
    virtual auto IsValid() const noexcept -> bool { return {}; }
    virtual auto LastConnected() const noexcept -> Time { return {}; }
    virtual auto Port() const noexcept -> std::uint16_t { return {}; }
    auto PreviousLastConnected() const noexcept -> Time override { return {}; }
    auto PreviousServices() const noexcept -> Set<bitcoin::Service> override
    {
        return {};
    }
    auto Serialize(proto::BlockchainPeerAddress&) const noexcept
        -> bool override
    {
        return {};
    }
    virtual auto Services() const noexcept -> Set<bitcoin::Service>
    {
        return {};
    }
    virtual auto Style() const noexcept -> Protocol { return {}; }
    virtual auto Type() const noexcept -> Transport { return {}; }

    auto AddService(const bitcoin::Service) noexcept -> void override {}
    auto RemoveService(const bitcoin::Service) noexcept -> void override {}
    auto SetIncoming(bool) noexcept -> void override {}
    auto SetLastConnected(const Time&) noexcept -> void override {}
    auto SetServices(const Set<bitcoin::Service>&) noexcept -> void override {}

    AddressPrivate() noexcept = default;
    AddressPrivate(const AddressPrivate&) = delete;
    AddressPrivate(AddressPrivate&&) = delete;
    auto operator=(const AddressPrivate&) -> AddressPrivate& = delete;
    auto operator=(AddressPrivate&&) -> AddressPrivate& = delete;

    ~AddressPrivate() override = default;
};
}  // namespace opentxs::network::blockchain
