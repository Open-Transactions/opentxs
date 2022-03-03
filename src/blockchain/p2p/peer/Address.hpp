// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain

class Data;
class Identifier;

using OTData = Pimpl<Data>;
using OTIdentifier = Pimpl<Identifier>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::peer
{
class Address
{
public:
    using pointer = std::unique_ptr<internal::Address>;

    auto Bytes() const noexcept -> OTData;
    auto Chain() const noexcept -> blockchain::Type;
    auto Display() const noexcept -> UnallocatedCString;
    auto ID() const noexcept -> OTIdentifier;
    auto Incoming() const noexcept -> bool;
    auto Port() const noexcept -> std::uint16_t;
    auto Services() const noexcept -> UnallocatedSet<Service>;
    auto Type() const noexcept -> Network;

    auto UpdateServices(const UnallocatedSet<p2p::Service>& services) noexcept
        -> pointer;
    auto UpdateTime(const Time& time) noexcept -> pointer;

    Address(pointer address) noexcept;

private:
    mutable std::mutex lock_;
    pointer address_;
};
}  // namespace opentxs::blockchain::p2p::peer
