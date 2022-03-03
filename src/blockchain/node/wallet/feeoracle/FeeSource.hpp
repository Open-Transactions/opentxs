// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/json.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <optional>
#include <random>

#include "core/Worker.hpp"
#include "internal/blockchain/node/wallet/FeeSource.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Allocated.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace json
{
class value;
}  // namespace json
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace display
{
class Scale;
}  // namespace display

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Amount;
class Timer;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeSource::Imp
    : public opentxs::implementation::Allocated,
      public Worker<Imp, api::Session>
{
public:
    const CString hostname_;
    const CString path_;
    const bool https_;
    const CString asio_;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        query = OT_ZMQ_INTERNAL_SIGNAL + 0,
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    auto Shutdown() noexcept -> void;

    ~Imp() override;

protected:
    auto process_double(double rate, unsigned long long int scale) noexcept
        -> std::optional<Amount>;
    auto process_int(std::int64_t rate, unsigned long long int scale) noexcept
        -> std::optional<Amount>;

    Imp(const api::Session& api,
        CString endpoint,
        CString hostname,
        CString path,
        bool https,
        allocator_type&& alloc) noexcept;

private:
    friend Worker<Imp, api::Session>;

    static const display::Scale scale_;

    std::random_device rd_;
    std::default_random_engine eng_;
    std::uniform_int_distribution<int> dist_;
    network::zeromq::socket::Raw to_oracle_;
    std::optional<std::future<boost::json::value>> future_;
    Timer timer_;

    auto jitter() noexcept -> std::chrono::seconds;
    virtual auto process(const boost::json::value& data) noexcept
        -> std::optional<Amount> = 0;

    auto pipeline(network::zeromq::Message&&) noexcept -> void;
    auto query() noexcept -> void;
    auto reset_timer() noexcept -> void;
    auto shutdown(std::promise<void>&) noexcept -> void;
    auto startup() noexcept -> void;
    auto state_machine() noexcept -> bool;
};
