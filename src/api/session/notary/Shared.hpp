// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <functional>
#include <shared_mutex>

#include "internal/api/session/notary/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/otx/blind/Mint.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::notary
{
class Shared final : public opentxs::Allocated
{
public:
    using Queue = Deque<identifier::UnitDefinition>;
    using Map = opentxs::Map<identifier::UnitDefinition, MintSeries>;
    using GuardedQueue = libguarded::ordered_guarded<Queue, std::shared_mutex>;
    using GuardedData = libguarded::shared_guarded<Map, std::shared_mutex>;
    using GuardedSocket =
        libguarded::plain_guarded<opentxs::network::zeromq::socket::Raw>;

    const opentxs::network::zeromq::BatchID batch_id_;
    const CString endpoint_;
    GuardedQueue queue_;
    GuardedData data_;
    GuardedSocket to_actor_;

    auto get_allocator() const noexcept -> allocator_type final;

    Shared(const opentxs::network::zeromq::Context& zmq) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;
};
}  // namespace opentxs::api::session::notary
