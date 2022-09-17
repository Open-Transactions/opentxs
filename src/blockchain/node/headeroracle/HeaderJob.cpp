// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/headeroracle/HeaderJob.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
HeaderJob::Imp::Imp(
    bool valid,
    Vector<block::Hash>&& previous,
    const api::Session* api,
    std::string_view endpoint) noexcept
    : valid_(valid && (nullptr != api))
    , previous_(std::move(previous))
    , to_parent_([&]() -> std::optional<network::zeromq::socket::Raw> {
        if (nullptr != api) {
            using Type = network::zeromq::socket::Type;
            auto out = api->Network().ZeroMQ().Internal().RawSocket(Type::Push);
            const auto rc = out.Connect(endpoint.data());

            OT_ASSERT(rc);

            return out;
        } else {

            return std::nullopt;
        }
    }())
{
}

HeaderJob::Imp::Imp() noexcept
    : Imp(false, {}, nullptr, {})
{
}

HeaderJob::Imp::~Imp()
{
    if (to_parent_.has_value()) {
        using Job = headeroracle::Job;
        to_parent_->SendDeferred(
            MakeWork(Job::job_finished), __FILE__, __LINE__);
    }
}
}  // namespace opentxs::blockchain::node::internal

namespace opentxs::blockchain::node::internal
{
HeaderJob::HeaderJob(std::unique_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
}

HeaderJob::HeaderJob() noexcept
    : HeaderJob(std::make_unique<Imp>())
{
}

HeaderJob::HeaderJob(HeaderJob&& rhs) noexcept
    : HeaderJob(std::move(rhs.imp_))
{
}

auto HeaderJob::IsValid() const noexcept -> bool
{
    if (imp_) {

        return imp_->valid_;
    } else {

        return false;
    }
}

auto HeaderJob::operator=(HeaderJob&& rhs) noexcept -> HeaderJob&
{
    using std::swap;
    swap(imp_, rhs.imp_);

    return *this;
}

auto HeaderJob::Recent() const noexcept -> const Headers&
{
    if (imp_) {

        return imp_->previous_;
    } else {
        static const auto blank = Vector<block::Hash>{};

        return blank;
    }
}

HeaderJob::~HeaderJob() = default;
}  // namespace opentxs::blockchain::node::internal