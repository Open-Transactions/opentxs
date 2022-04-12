// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/network/p2p/State.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Legacy;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::internal
{
class Blockchain
{
public:
    virtual auto BlockAvailableEndpoint() const noexcept
        -> std::string_view = 0;
    virtual auto BlockQueueUpdateEndpoint() const noexcept
        -> std::string_view = 0;
    virtual auto Database() const noexcept
        -> const opentxs::blockchain::database::common::Database& = 0;
    virtual auto FilterUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    using SyncData = UnallocatedVector<opentxs::network::p2p::State>;
    virtual auto Hello() const noexcept -> SyncData = 0;
    virtual auto IsEnabled(const opentxs::blockchain::Type chain) const noexcept
        -> bool = 0;
    virtual auto Mempool() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto PeerUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto PublishStartup(
        const opentxs::blockchain::Type chain,
        OTZMQWorkType type) const noexcept -> bool = 0;
    virtual auto Reorg() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto ReportProgress(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height current,
        const opentxs::blockchain::block::Height target) const noexcept
        -> void = 0;
    virtual auto RestoreNetworks() const noexcept -> void = 0;
    virtual auto SyncEndpoint() const noexcept -> std::string_view = 0;
    virtual auto UpdatePeer(
        const opentxs::blockchain::Type chain,
        const UnallocatedCString& address) const noexcept -> void = 0;

    virtual auto Init(
        const api::crypto::Blockchain& crypto,
        const api::Legacy& legacy,
        const UnallocatedCString& dataFolder,
        const Options& args) noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    virtual ~Blockchain() = default;

protected:
    Blockchain() = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace opentxs::api::network::internal
