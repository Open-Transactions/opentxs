// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/integration/Helpers.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <span>
#include <utility>

#include "internal/api/Settings.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
const User IntegrationFixture::alex_{
    "spike nominee miss inquiry fee nothing belt list other daughter leave "
    "valley twelve gossip paper",
    "Alex"};
const User IntegrationFixture::bob_{
    "trim thunder unveil reduce crop cradle zone inquiry anchor skate property "
    "fringe obey butter text tank drama palm guilt pudding laundry stay axis "
    "prosper",
    "Bob"};
const User IntegrationFixture::issuer_{
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon about",
    "Issuer"};
const User IntegrationFixture::chris_{
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon "
    "abandon abandon prosper",
    "Chris"};
const Server IntegrationFixture::server_1_{};

auto set_introduction_server(
    const ot::api::session::Client& api,
    const Server& server) noexcept -> void
{
    auto bytes = ot::Space{};
    server.Contract()->Serialize(ot::writer(bytes), true);
    auto clientVersion = api.Wallet().Internal().Server(ot::reader(bytes));
    api.OTX().SetIntroductionServer(clientVersion);
}

auto test_future(std::future<bool>& future, const unsigned int seconds) noexcept
    -> bool
{
    const auto status =
        future.wait_until(ot::Clock::now() + std::chrono::seconds{seconds});

    if (std::future_status::ready == status) { return future.get(); }

    return false;
}

Callbacks::Callbacks(
    const ot::api::Context& api,
    const ot::UnallocatedCString& name) noexcept
    : api_(api)
    , callback_lock_()
    , callback_(ot::network::zeromq::ListenCallback::Factory(
          [this](auto&& incoming) -> void { callback(std::move(incoming)); }))
    , map_lock_()
    , name_(name)
    , widget_map_()
    , ui_names_()
{
}

auto Callbacks::callback(ot::network::zeromq::Message&& incoming) noexcept
    -> void
{
    ot::Lock lock(callback_lock_);
    const auto widgetID =
        api_.Factory().IdentifierFromBase58(incoming.Payload()[0].Bytes());

    ASSERT_FALSE(widgetID.asBase58(ot::Context().Crypto()).empty());

    auto& [type, counter, callbackData] = widget_map_.at(widgetID);
    auto& [limit, callback, future] = callbackData;
    ++counter;

    if (counter >= limit) {
        if (callback) {
            future.set_value(callback());
            callback = {};
            future = {};
            limit = 0;
        } else {
            ot::LogError()(OT_PRETTY_CLASS())(name_)(" missing callback for ")(
                static_cast<int>(type))
                .Flush();
        }
    } else {
        ot::LogVerbose()(OT_PRETTY_CLASS())("Skipping update ")(
            counter)(" to ")(static_cast<int>(type))
            .Flush();
    }
}

auto Callbacks::Count() const noexcept -> std::size_t
{
    ot::Lock lock(map_lock_);

    return widget_map_.size();
}

auto Callbacks::RegisterWidget(
    const ot::Lock& callbackLock,
    const Widget type,
    const ot::identifier::Generic& id,
    int counter,
    WidgetCallback callback) noexcept -> std::future<bool>
{
    ot::LogDetail()("::Callbacks::")(__func__)(": Name: ")(name_)(" ID: ")(id)
        .Flush();
    WidgetData data{};
    std::get<0>(data) = type;
    auto& [limit, cb, promise] = std::get<2>(data);
    limit = counter;
    cb = callback;
    promise = {};
    auto output = promise.get_future();
    widget_map_.emplace(id, std::move(data));
    ui_names_.emplace(type, id);

    OT_ASSERT(widget_map_.size() == ui_names_.size());

    return output;
}

auto Callbacks::SetCallback(
    const Widget type,
    int limit,
    WidgetCallback callback) noexcept -> std::future<bool>
{
    ot::Lock lock(map_lock_);
    auto& [counter, cb, promise] =
        std::get<2>(widget_map_.at(ui_names_.at(type)));
    counter += limit;
    cb = callback;
    promise = {};

    return promise.get_future();
}

Issuer::Issuer() noexcept
    : bailment_counter_(0)
    , bailment_promise_()
    , bailment_(bailment_promise_.get_future())
    , store_secret_promise_()
    , store_secret_(store_secret_promise_.get_future())
{
}

auto Server::Contract() const noexcept -> ot::OTServerContract
{
    return api_->Wallet().Internal().Server(id_);
}

auto Server::Reason() const noexcept -> ot::PasswordPrompt
{
    OT_ASSERT(nullptr != api_);

    return api_->Factory().PasswordPrompt(__func__);
}

auto Server::init(const ot::api::session::Notary& api) noexcept -> void
{
    if (init_) { return; }

    api_ = &api;
    const_cast<ot::identifier::Notary&>(id_) = api.ID();

    {
        const auto section = ot::String::Factory("permissions");
        const auto key = ot::String::Factory("admin_password");
        auto value = ot::String::Factory();
        auto exists{false};
        api.Config().Internal().Check_str(section, key, value, exists);

        OT_ASSERT(exists);

        const_cast<ot::UnallocatedCString&>(password_) = value->Get();
    }

    OT_ASSERT(false == id_.empty());
    OT_ASSERT(false == password_.empty());

    init_ = true;
}
}  // namespace ottest
