// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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

class BlockchainHandle;
}  // namespace network
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
class Stats;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
/**
 The api::network::Blockchain API is used for accessing blockchain-related
 network functionality.
 */
class OPENTXS_EXPORT Blockchain
{
public:
    using Chain = opentxs::blockchain::Type;

    virtual auto Disable(const Chain type) const noexcept -> bool = 0;
    virtual auto Enable(const Chain type, const std::string_view seednode = {})
        const noexcept -> bool = 0;
    virtual auto EnabledChains(alloc::Default alloc = {}) const noexcept
        -> Set<Chain> = 0;
    /// throws std::out_of_range if chain has not been started
    virtual auto GetChain(const Chain type) const noexcept(false)
        -> BlockchainHandle = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Blockchain& = 0;
    virtual auto Profile() const noexcept -> BlockchainProfile = 0;
    virtual auto Start(const Chain type, const std::string_view seednode = {})
        const noexcept -> bool = 0;
    virtual auto Stats() const noexcept -> opentxs::blockchain::node::Stats = 0;
    virtual auto Stop(const Chain type) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Blockchain& = 0;

    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;

    OPENTXS_NO_EXPORT virtual ~Blockchain() = default;

protected:
    Blockchain() = default;
};
}  // namespace opentxs::api::network
