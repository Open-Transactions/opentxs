// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/feesource/FeeSource.hpp"  // IWYU pragma: associated
#include "internal/blockchain/node/wallet/Factory.hpp"  // IWYU pragma: associated

#include <exception>
#include <ratio>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::factory
{
auto FeeSources(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const blockchain::node::Manager> node) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(node);

    if (api->GetOptions().TestMode()) { return; }

    switch (node->Internal().Chain()) {
        case blockchain::Type::Bitcoin: {
            BTCFeeSources(std::move(api), std::move(node));
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::wallet
{
auto print(FeeSourceJobs job) noexcept -> std::string_view
{
    try {
        using Job = FeeSourceJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::query, "query"},
            {Job::init, "init"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid FeeSourceJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

auto FeeSource::Imp::display_scale() -> const display::Scale&
{
    static auto scale = display::Scale{"", "", {{10, 0}}, 0, 3};
    return scale;
}

FeeSource::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view hostname,
    std::string_view path,
    bool https,
    CString&& asio,
    network::zeromq::BatchID batch,
    allocator_type&& alloc) noexcept
    : Actor(
          *api,
          LogTrace(),
          [&] {
              auto out = CString{print(node->Internal().Chain()), alloc};
              out.append(" fee source: ");
              out.append(hostname);

              return out;
          }(),
          0ms,
          batch,
          alloc,
          {
              {asio, Bind},
              {api->Endpoints().Shutdown(), Connect},
              {node->Internal().Endpoints().shutdown_publish_, Connect},
          },
          {},
          {},
          {
              {Push,
               Internal,
               {
                   {node->Internal().Endpoints().fee_oracle_pull_, Connect},
               }},
          })
    , asio_(std::move(asio), alloc)
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , api_(*api_p_)
    , node_(*node_p_)
    , hostname_(hostname, alloc)
    , path_(path, alloc)
    , https_(https)
    , rd_()
    , eng_(rd_())
    , dist_(-60, 60)
    , to_oracle_(pipeline_.Internal().ExtraSocket(0))
    , future_(std::nullopt)
    , timer_(api_.Network().Asio().Internal().GetTimer())
{
}

FeeSource::Imp::Imp(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::string_view hostname,
    std::string_view path,
    bool https,
    network::zeromq::BatchID batch,
    allocator_type&& alloc) noexcept
    : Imp(std::move(api),
          std::move(node),
          std::move(hostname),
          std::move(path),
          std::move(https),
          network::zeromq::MakeArbitraryInproc(alloc),
          std::move(batch),
          std::move(alloc))
{
}

auto FeeSource::Imp::do_shutdown() noexcept -> void
{
    timer_.Cancel();
    node_p_.reset();
    api_p_.reset();
}

auto FeeSource::Imp::do_startup(alloc::Strategy) noexcept -> bool
{
    if (api_.Internal().ShuttingDown() || node_.Internal().ShuttingDown()) {

        return true;
    }

    query();
    reset_timer();

    return false;
}

auto FeeSource::Imp::jitter() noexcept -> std::chrono::seconds
{
    return std::chrono::seconds{dist_(eng_)};
}

auto FeeSource::Imp::pipeline(
    const Work work,
    Message&& msg,
    alloc::Strategy) noexcept -> void
{
    switch (work) {
        case Work::query: {
            query();
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
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
    log_(OT_PRETTY_CLASS())(name_)(": obtained scaled amount ")(
        display_scale().Format(value))(" from raw input ")(
        rate)(" and scale value ")(scale)
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
    log_(OT_PRETTY_CLASS())(name_)(": obtained scaled amount ")(
        display_scale().Format(value))(" from raw input ")(
        rate)(" and scale value ")(scale)
        .Flush();

    if (0 > value) {

        return std::nullopt;
    } else {

        return out;
    }
}

auto FeeSource::Imp::query() noexcept -> void
{
    // NOTE Work::statemachine will be received after json is fetched
    future_ = api_.Network().Asio().Internal().FetchJson(
        hostname_, path_, https_, asio_);
}

auto FeeSource::Imp::reset_timer() noexcept -> void
{
    static constexpr auto interval = std::chrono::minutes{15};
    timer_.SetRelative(interval + jitter());
    timer_.Wait([this](const auto& ec) {
        if (ec) {
            if (unexpected_asio_error(ec)) {
                LogError()(OT_PRETTY_CLASS())(name_)(": received asio error (")(
                    ec.value())(") :")(ec)
                    .Flush();
            }
        } else {
            pipeline_.Push(MakeWork(Work::query));
        }
    });
}

auto FeeSource::Imp::work(alloc::Strategy monotonic) noexcept -> bool
{
    auto& future = future_.value();
    static constexpr auto limit = 25ms;
    static constexpr auto ready = std::future_status::ready;

    try {
        if (const auto status = future.wait_for(limit); status == ready) {
            if (const auto data = process(future.get()); data.has_value()) {
                to_oracle_.SendDeferred(
                    [&] {
                        auto out = MakeWork(FeeOracleJobs::update_estimate);
                        data->Serialize(out.AppendBytes());

                        return out;
                    }(),
                    __FILE__,
                    __LINE__);
            }

            reset_timer();

            return false;
        } else {
            LogError()(OT_PRETTY_CLASS())(name_)(": future is not ready")
                .Flush();

            return true;
        }
    } catch (const std::exception& e) {
        LogError()(name_)(": ")(e.what()).Flush();

        return false;
    }
}

FeeSource::Imp::~Imp() = default;
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
FeeSource::FeeSource(boost::shared_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(imp_);
}

auto FeeSource::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

FeeSource::~FeeSource() = default;
}  // namespace opentxs::blockchain::node::wallet
