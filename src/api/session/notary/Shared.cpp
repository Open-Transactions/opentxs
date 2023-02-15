// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/session/notary/Shared.hpp"  // IWYU pragma: associated

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"

namespace opentxs::api::session::notary
{
Shared::Shared(const opentxs::network::zeromq::Context& zmq) noexcept
    : batch_id_(zmq.Internal().PreallocateBatch())
    , endpoint_([&] {
        auto* mr = zmq.Internal().Alloc(batch_id_);

        return opentxs::network::zeromq::MakeArbitraryInproc({mr});
    }())
    , queue_()
    , data_()
    , to_actor_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = zmq.Internal().RawSocket(Type::Push);
        const auto rc = out.Bind(endpoint_.data());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto Shared::get_allocator() const noexcept -> allocator_type
{
    return endpoint_.get_allocator();
}

Shared::~Shared() = default;
}  // namespace opentxs::api::session::notary
