// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Hash;
class Transaction;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class Parser
{
public:
    [[nodiscard]] static auto Check(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const Hash& expected,
        const ReadView bytes,
        alloc::Strategy alloc) noexcept -> bool;
    [[nodiscard]] static auto Construct(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const ReadView bytes,
        Block& out,
        alloc::Strategy alloc) noexcept -> bool;
    [[nodiscard]] static auto Construct(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const Hash& expected,
        const ReadView bytes,
        Block& out,
        alloc::Strategy alloc) noexcept -> bool;
    [[nodiscard]] static auto Construct(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const network::zeromq::Message& message,
        Vector<Block>& out,
        alloc::Strategy alloc) noexcept -> bool;
    [[nodiscard]] static auto Transaction(
        const api::Crypto& crypto,
        const blockchain::Type type,
        const std::size_t position,
        const Time& time,
        const ReadView bytes,
        block::Transaction& out,
        alloc::Strategy alloc) noexcept -> bool;

    virtual ~Parser() = default;

private:
    [[nodiscard]] virtual auto GetHeader() const noexcept -> ReadView = 0;

    [[nodiscard]] virtual auto operator()(
        const Hash& expected,
        ReadView bytes) && noexcept -> bool = 0;
    [[nodiscard]] virtual auto operator()(ReadView bytes, Hash& out) noexcept
        -> bool = 0;
    [[nodiscard]] virtual auto operator()(
        const Hash& expected,
        ReadView bytes,
        Block& out) && noexcept -> bool = 0;
    [[nodiscard]] virtual auto operator()(
        const std::size_t position,
        const Time& time,
        ReadView bytes,
        block::Transaction& out) && noexcept -> bool = 0;
};
}  // namespace opentxs::blockchain::block
