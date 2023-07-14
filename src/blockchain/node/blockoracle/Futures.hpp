// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Futures final : public opentxs::Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
    auto Queue(const block::Hash& hash, BlockResult& out) noexcept -> void;
    auto Receive(
        const api::Crypto& crypto,
        const blockchain::Type chain,
        const block::Hash& hash,
        const BlockLocation& location,
        allocator_type monotonic) noexcept -> void;

    Futures(
        const api::Session& api,
        const std::string_view name,
        blockchain::Type chain,
        allocator_type alloc) noexcept;
    Futures() = delete;
    Futures(const Futures&) = delete;
    Futures(Futures&&) = delete;
    auto operator=(const Futures&) -> Futures& = delete;
    auto operator=(Futures&&) -> Futures& = delete;

    ~Futures() final;

private:
    const Log& log_;
    const std::string_view name_;
    const blockchain::Type chain_;
    Requests requests_;
    Index pending_;
    network::zeromq::socket::Raw to_blockchain_api_;
};
}  // namespace opentxs::blockchain::node::blockoracle
