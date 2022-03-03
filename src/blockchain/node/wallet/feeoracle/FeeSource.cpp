// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/feeoracle/FeeSource.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/Factory.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <atomic>
#include <exception>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto FeeSources(
    const api::Session& api,
    const blockchain::Type chain,
    const std::string_view endpoint,
    alloc::Resource* mr) noexcept
    -> ForwardList<blockchain::node::wallet::FeeSource>
{
    if (nullptr == mr) { mr = alloc::System(); }

    using ReturnType = blockchain::node::wallet::FeeSource::Imp;
    auto alloc = ReturnType::allocator_type{mr};
    auto out = ForwardList<blockchain::node::wallet::FeeSource>{alloc};

    switch (chain) {
        case blockchain::Type::Bitcoin: {

            return BTCFeeSources(api, endpoint, mr);
        }
        default: {

            return out;
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::wallet
{
const display::Scale FeeSource::Imp::scale_{"", "", {{10, 0}}, 0, 3};

FeeSource::Imp::Imp(
    const api::Session& api,
    CString endpoint,
    CString hostname,
    CString path,
    bool https,
    allocator_type&& alloc) noexcept
    : Allocated(std::move(alloc))
    , Worker(api, {})
    , hostname_(std::move(hostname))
    , path_(std::move(path))
    , https_(https)
    , asio_(network::zeromq::MakeArbitraryInproc(allocator_.resource()))
    , rd_()
    , eng_(rd_())
    , dist_(-60, 60)
    , to_oracle_([&] {
        auto out = factory::ZMQSocket(
            api_.Network().ZeroMQ(),
            opentxs::network::zeromq::socket::Type::Publish);
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , future_(std::nullopt)
    , timer_(api.Network().Asio().Internal().GetTimer())
{
    pipeline_.BindSubscriber(asio_, [](auto) { return MakeWork(Work::init); });
}

auto FeeSource::Imp::jitter() noexcept -> std::chrono::seconds
{
    return std::chrono::seconds{dist_(eng_)};
}

auto FeeSource::Imp::pipeline(network::zeromq::Message&& in) noexcept -> void
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
        case Work::query: {
            query();
        } break;
        case Work::init: {
            startup();
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

auto FeeSource::Imp::process_double(
    double rate,
    unsigned long long int scale) noexcept -> std::optional<Amount>
{
    auto out = std::optional<Amount>{
        static_cast<std::int64_t>(rate * static_cast<double>(scale))};
    const auto& value = out.value();
    LogTrace()(OT_PRETTY_CLASS())("obtained scaled amount ")(scale_.Format(
        value))(" from raw input ")(rate)(" and scale value ")(scale)
        .Flush();

    if (0 > value) {

        return std::nullopt;
    } else {

        return out;
    }
}

auto FeeSource::Imp::process_int(
    std::int64_t rate,
    unsigned long long int scale) noexcept -> std::optional<Amount>
{
    auto out = std::optional<Amount>{static_cast<std::int64_t>(rate * scale)};
    const auto& value = out.value();
    LogTrace()(OT_PRETTY_CLASS())("obtained scaled amount ")(scale_.Format(
        value))(" from raw input ")(rate)(" and scale value ")(scale)
        .Flush();

    if (0 > value) {

        return std::nullopt;
    } else {

        return out;
    }
}

auto FeeSource::Imp::query() noexcept -> void
{
    future_ = api_.Network().Asio().Internal().FetchJson(
        hostname_, path_, https_, asio_);
}

auto FeeSource::Imp::reset_timer() noexcept -> void
{
    static constexpr auto interval = std::chrono::minutes{15};
    timer_.SetRelative(interval + jitter());
    timer_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            pipeline_.Push(MakeWork(Work::query));
        }
    });
}

auto FeeSource::Imp::startup() noexcept -> void
{
    query();
    reset_timer();
}

auto FeeSource::Imp::state_machine() noexcept -> bool
{
    if (false == future_.has_value()) { return false; }

    auto& future = future_.value();
    static constexpr auto limit = 25ms;
    static constexpr auto ready = std::future_status::ready;

    try {
        if (const auto status = future.wait_for(limit); status == ready) {
            if (const auto data = process(future.get()); data.has_value()) {
                to_oracle_.Send([&] {
                    auto out = MakeWork(OT_ZMQ_INTERNAL_SIGNAL);
                    data->Serialize(out.AppendBytes());

                    return out;
                }());
            }

            reset_timer();

            return false;
        } else {
            LogError()(OT_PRETTY_CLASS())("Future is not ready").Flush();

            return true;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto FeeSource::Imp::Shutdown() noexcept -> void { signal_shutdown(); }

auto FeeSource::Imp::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (auto previous = running_.exchange(false); previous) {
        timer_.Cancel();
        pipeline_.Close();
        promise.set_value();
    }
}

FeeSource::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
FeeSource::FeeSource(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp);
}

FeeSource::FeeSource(FeeSource&& rhs, allocator_type alloc) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;

    OT_ASSERT(imp_->get_allocator() == alloc);
}

auto FeeSource::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto FeeSource::Shutdown() noexcept -> void { return imp_->Shutdown(); }

FeeSource::~FeeSource()
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
