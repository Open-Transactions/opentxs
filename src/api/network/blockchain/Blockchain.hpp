// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/Stats.hpp"

#pragma once

#include <string_view>

#include "internal/api/network/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
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
class BlockchainHandle;
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::implementation
{
class Blockchain final : public network::Blockchain
{
public:
    struct Imp;

    using Chain = opentxs::blockchain::Type;
    using Endpoints = Vector<CString>;

    auto Disable(const Chain type) const noexcept -> bool final;
    auto Enable(const Chain type, const std::string_view seednode = "")
        const noexcept -> bool final;
    auto EnabledChains(alloc::Default alloc = {}) const noexcept
        -> Set<Chain> final;
    /// throws std::out_of_range if chain has not been started
    auto GetChain(const Chain type) const noexcept(false)
        -> BlockchainHandle final;
    auto Internal() const noexcept -> const internal::Blockchain& final;
    auto Profile() const noexcept -> BlockchainProfile final;
    auto Start(const Chain type, const std::string_view seednode = "")
        const noexcept -> bool final;
    auto Stats() const noexcept -> opentxs::blockchain::node::Stats final;
    auto Stop(const Chain type) const noexcept -> bool final;

    auto Internal() noexcept -> internal::Blockchain& final;

    Blockchain(Imp* imp) noexcept;
    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;

    ~Blockchain() final;

private:
    Imp* imp_;
};
}  // namespace opentxs::api::network::implementation
