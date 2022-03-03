// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/Factory.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/FeeOracle.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <new>
#include <numeric>  // IWYU pragma: keep

#include "blockchain/node/wallet/feeoracle/FeeOracle.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/wallet/FeeSource.hpp"
#include "internal/core/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto FeeOracle(
    const api::Session& api,
    const blockchain::Type chain,
    alloc::Resource* mr) noexcept -> blockchain::node::wallet::FeeOracle
{
    if (nullptr == mr) { mr = alloc::System(); }

    using ReturnType = blockchain::node::wallet::FeeOracle::Imp;
    auto alloc = ReturnType::allocator_type{mr};
    auto* resource = alloc.resource();
    auto* out = resource->allocate(sizeof(ReturnType), alignof(ReturnType));

    return new (out) ReturnType{
        api,
        chain,
        std::move(alloc),
        network::zeromq::MakeArbitraryInproc(resource)};
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::wallet
{
FeeOracle::Imp::Imp(
    const api::Session& api,
    const blockchain::Type chain,
    allocator_type&& alloc,
    CString endpoint) noexcept
    : Allocated(std::move(alloc))
    , Worker(api, {})
    , chain_(chain)
    , timer_(api.Network().Asio().Internal().GetTimer())
    , sources_(factory::FeeSources(api, chain_, endpoint, alloc.resource()))
    , data_(alloc.resource())
    , output_(std::nullopt)
{
    pipeline_.BindSubscriber(endpoint);
    reset_timer();
}

auto FeeOracle::Imp::EstimatedFee() const noexcept -> std::optional<Amount>
{
    return *output_.lock_shared();
}

auto FeeOracle::Imp::pipeline(network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_.load()) {
        shutdown(shutdown_promise_);

        return;
    }

    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Work::update_estimate: {
            process_update(std::move(in));
            do_work();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto FeeOracle::Imp::process_update(network::zeromq::Message&& in) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    try {
        data_.emplace_back(Clock::now(), opentxs::factory::Amount(body.at(1)));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto FeeOracle::Imp::reset_timer() noexcept -> void
{
    static constexpr auto interval = std::chrono::minutes{1};
    timer_.SetRelative(interval);
    timer_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            trigger();
            reset_timer();
        }
    });
}

auto FeeOracle::Imp::state_machine() noexcept -> bool
{
    const auto sum = [this] {
        static constexpr auto validity = std::chrono::minutes{20};
        const auto limit = Clock::now() - validity;
        auto out = Amount{0};
        std::remove_if(data_.begin(), data_.end(), [&](const auto& v) {
            if (v.first < limit) {

                return true;
            } else {
                out += v.second;

                return false;
            }
        });

        return out;
    }();
    output_.modify_detach([this,
                           average =
                               sum / std::max<std::size_t>(data_.size(), 1u)](
                              auto& value) mutable {
        if (0 < average) {
            static const auto scale = display::Scale{"", "", {{10, 0}}, 0, 0};
            LogDetail()("Updated ")(DisplayString(chain_))(" fee estimate to ")(
                scale.Format(average))(" sat / 1000 vBytes")
                .Flush();
            value.emplace(std::move(average));
        } else {
            LogDetail()("Fee estimate for ")(DisplayString(chain_))(
                " not available")
                .Flush();
            value = std::nullopt;
        }
    });

    return false;
}

auto FeeOracle::Imp::Shutdown() noexcept -> void { signal_shutdown(); }

auto FeeOracle::Imp::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        for (auto& source : sources_) { source.Shutdown(); }

        timer_.Cancel();
        pipeline_.Close();
        promise.set_value();
    }
}

FeeOracle::Imp::~Imp() { signal_shutdown().get(); }
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
FeeOracle::FeeOracle(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

auto FeeOracle::EstimatedFee() const noexcept -> std::optional<Amount>
{
    return imp_->EstimatedFee();
}

auto FeeOracle::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto FeeOracle::Shutdown() noexcept -> void { imp_->Shutdown(); }

FeeOracle::~FeeOracle()
{
    if (nullptr != imp_) {
        auto alloc = imp_->get_allocator();
        // TODO c++20 use delete_object
        imp_->~Imp();
        alloc.resource()->deallocate(imp_, sizeof(Imp), alignof(Imp));
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node::wallet
