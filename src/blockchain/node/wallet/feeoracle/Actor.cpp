// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "blockchain/node/wallet/feeoracle/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
#include <exception>
#include <memory>
#include <numeric>  // IWYU pragma: keep
#include <optional>
#include <ratio>
#include <string_view>

#include "blockchain/node/wallet/feeoracle/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Factory.hpp"
#include "internal/core/Factory.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
FeeOracle::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : opentxs::Actor<FeeOracle::Actor, FeeOracleJobs>(
          *api,
          LogTrace(),
          [&] {
              auto out = CString{print(node->Internal().Chain()), alloc};
              out.append(" fee oracle");

              return out;
          }(),
          0ms,
          batch,
          alloc,
          [&] {
              auto out = network::zeromq::EndpointArgs{alloc};
              out.emplace_back(api->Endpoints().Shutdown(), Direction::Connect);
              out.emplace_back(
                  node->Internal().Endpoints().shutdown_publish_,
                  Direction::Connect);

              return out;
          }(),
          [&] {
              auto out = network::zeromq::EndpointArgs{alloc};
              out.emplace_back(
                  node->Internal().Endpoints().fee_oracle_pull_,
                  Direction::Bind);

              return out;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , chain_(node_.Internal().Chain())
    , timer_(api_.Network().Asio().Internal().GetTimer())
    , data_(alloc)
    , output_(shared_p_->data_)
{
    OT_ASSERT(api_p_);
    OT_ASSERT(node_p_);
    OT_ASSERT(shared_p_);
}

auto FeeOracle::Actor::do_shutdown() noexcept -> void
{
    timer_.Cancel();
    shared_p_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto FeeOracle::Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || node_.Internal().ShuttingDown()) {

        return true;
    }

    factory::FeeSources(api_p_, node_p_);
    do_work(monotonic);

    return false;
}

auto FeeOracle::Actor::pipeline(
    const Work work,
    network::zeromq::Message&& in,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::update_estimate: {
            process_update(std::move(in), monotonic);
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())("unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto FeeOracle::Actor::process_update(
    network::zeromq::Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    try {
        data_.emplace_back(Clock::now(), opentxs::factory::Amount(body.at(1)));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    do_work(monotonic);
}

auto FeeOracle::Actor::work(allocator_type monotonic) noexcept -> bool
{
    const auto sum = [this] {
        static constexpr auto validity = std::chrono::minutes{20};
        const auto limit = Clock::now() - validity;
        auto out = Amount{0};
        data_.erase(
            std::remove_if(
                data_.begin(),
                data_.end(),
                [&](const auto& v) {
                    if (v.first < limit) {

                        return true;
                    } else {
                        out += v.second;

                        return false;
                    }
                }),
            data_.end());

        return out;
    }();
    output_.modify_detach([this, average = sum / std::max(data_.size(), 1_uz)](
                              auto& value) mutable {
        if (0 < average) {
            static const auto scale = display::Scale{"", "", {{10, 0}}, 0, 0};
            log_(OT_PRETTY_CLASS())(name_)(": Updated ")(print(chain_))(
                " fee estimate to ")(scale.Format(average))(
                " sat / 1000 vBytes")
                .Flush();
            value.emplace(std::move(average));
        } else {
            log_(OT_PRETTY_CLASS())(name_)(": Fee estimate for ")(
                print(chain_))(" not available")
                .Flush();
            value = std::nullopt;
        }
    });
    reset_timer(1min, timer_, Work::statemachine);

    return false;
}

FeeOracle::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::wallet
