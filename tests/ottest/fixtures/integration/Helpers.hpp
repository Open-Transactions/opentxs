// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <tuple>

#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/Mutex.hpp"
#include "ottest/Basic.hpp"
#include "ottest/fixtures/common/User.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Notary;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace ottest
{
class User;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
enum class Widget : int {
    AccountActivityUSD,
    AccountList,
    AccountSummaryBTC,
    AccountSummaryBCH,
    AccountSummaryUSD,
    ActivitySummary,
    ContactList,
    MessagableList,
    Profile,
    PayableListBTC,
    PayableListBCH,
    ActivityThreadAlice,
    ActivityThreadBob,
    ActivityThreadIssuer,
    ContactIssuer,
    AccountSummary,
};

using WidgetCallback = std::function<bool()>;
// target counter value, callback
using WidgetCallbackData = std::tuple<int, WidgetCallback, std::promise<bool>>;
// name, counter
using WidgetData = std::tuple<Widget, int, WidgetCallbackData>;
using WidgetMap = ot::UnallocatedMap<ot::identifier::Generic, WidgetData>;
using WidgetTypeMap = ot::UnallocatedMap<Widget, ot::identifier::Generic>;
using StateMap = std::map<
    ot::UnallocatedCString,
    ot::UnallocatedMap<Widget, ot::UnallocatedMap<int, WidgetCallback>>>;

struct Server {
    const ot::api::session::Notary* api_{nullptr};
    bool init_{false};
    const ot::identifier::Notary id_{};
    const ot::UnallocatedCString password_;

    auto Contract() const noexcept -> ot::OTServerContract;
    auto Reason() const noexcept -> ot::OTPasswordPrompt;

    auto init(const ot::api::session::Notary& api) noexcept -> void;
};

struct Callbacks {
    const ot::api::Context& api_;
    mutable std::mutex callback_lock_;
    ot::OTZMQListenCallback callback_;

    auto Count() const noexcept -> std::size_t;

    auto RegisterWidget(
        const ot::Lock& callbackLock,
        const Widget type,
        const ot::identifier::Generic& id,
        int counter = 0,
        WidgetCallback callback = {}) noexcept -> std::future<bool>;

    auto SetCallback(
        const Widget type,
        int limit,
        WidgetCallback callback) noexcept -> std::future<bool>;

    Callbacks(
        const ot::api::Context& api,
        const ot::UnallocatedCString& name) noexcept;
    Callbacks() = delete;

private:
    mutable std::mutex map_lock_;
    const ot::UnallocatedCString name_;
    WidgetMap widget_map_;
    WidgetTypeMap ui_names_;

    auto callback(ot::network::zeromq::Message&& incoming) noexcept -> void;
};

struct Issuer {
    static const int expected_bailments_{3};
    static const ot::UnallocatedCString new_notary_name_;

    int bailment_counter_;
    std::promise<bool> bailment_promise_;
    std::shared_future<bool> bailment_;
    std::promise<bool> store_secret_promise_;
    std::shared_future<bool> store_secret_;

    Issuer() noexcept;
};

class IntegrationFixture : public ::testing::Test
{
public:
    static const User alex_;
    static const User bob_;
    static const User issuer_;
    static const User chris_;
    static const Server server_1_;
};

auto set_introduction_server(
    const ot::api::session::Client& api,
    const Server& server) noexcept -> void;
auto test_future(
    std::future<bool>& future,
    const unsigned int seconds = 60) noexcept -> bool;
}  // namespace ottest
