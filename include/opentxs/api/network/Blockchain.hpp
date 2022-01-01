// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>
#include <string>
#include <vector>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class Blockchain;
}  // namespace internal
}  // namespace network
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::api::network
{
class OPENTXS_EXPORT Blockchain
{
public:
    struct Imp;

    using Chain = opentxs::blockchain::Type;
    using Endpoints = std::pmr::vector<std::string>;

    auto AddSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto ConnectedSyncServers() const noexcept -> Endpoints;
    auto DeleteSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type, const std::string& seednode = "")
        const noexcept -> bool;
    auto EnabledChains() const noexcept -> std::pmr::set<Chain>;
    /// throws std::out_of_range if chain has not been started
    auto GetChain(const Chain type) const noexcept(false)
        -> const opentxs::blockchain::node::Manager&;
    auto GetSyncServers() const noexcept -> Endpoints;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> internal::Blockchain&;
    auto Start(const Chain type, const std::string& seednode = "")
        const noexcept -> bool;
    auto StartSyncServer(
        const std::string& syncEndpoint,
        const std::string& publicSyncEndpoint,
        const std::string& updateEndpoint,
        const std::string& publicUpdateEndpoint) const noexcept -> bool;
    auto Stop(const Chain type) const noexcept -> bool;

    OPENTXS_NO_EXPORT Blockchain(Imp* imp) noexcept;

    OPENTXS_NO_EXPORT ~Blockchain();

private:
    Imp* imp_;

    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;
};
}  // namespace opentxs::api::network
