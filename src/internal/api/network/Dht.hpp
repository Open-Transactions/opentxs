// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include <functional>
#include <map>

#include "opentxs/api/network/Dht.hpp"
#include "opentxs/core/contract/Types.hpp"

namespace opentxs
{
namespace network
{
class OpenDHT;
}  // namespace network

namespace proto
{
class Nym;
class ServerContract;
class UnitDefinition;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::network::internal
{
class Dht : virtual public network::Dht
{
public:
    using NotifyCB = std::function<void(const std::string)>;
    using CallbackMap = std::pmr::map<contract::Type, NotifyCB>;

    auto Internal() const noexcept -> const internal::Dht& final
    {
        return *this;
    }
    using network::Dht::Insert;
    virtual auto Insert(const proto::Nym& nym) const noexcept -> void = 0;
    virtual auto Insert(const proto::ServerContract& contract) const noexcept
        -> void = 0;
    virtual auto Insert(const proto::UnitDefinition& contract) const noexcept
        -> void = 0;
    virtual auto OpenDHT() const noexcept
        -> const opentxs::network::OpenDHT& = 0;
    virtual auto RegisterCallbacks(const CallbackMap& callbacks) const noexcept
        -> void = 0;

    auto Internal() noexcept -> internal::Dht& final { return *this; }

    ~Dht() override = default;
};
}  // namespace opentxs::api::network::internal
