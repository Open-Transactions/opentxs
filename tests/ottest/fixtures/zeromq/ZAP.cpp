// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/ZAP.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <exception>
#include <functional>
#include <stdexcept>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/fixtures/zeromq/CustomZAPHandler.hpp"

namespace ottest
{
ZAP::ZAP() = default;

auto ZAP::MakeHandler(
    std::string_view domain,
    std::string_view endpoint,
    bool accept) noexcept -> std::shared_ptr<CustomZAPHandler>
{
    auto out =
        std::make_shared<CustomZAPHandler>(ot_, domain, endpoint, accept);
    ot_.ZAP().RegisterDomain(domain, endpoint);

    return out;
}

auto ZAP::SendRequest(
    std::string_view version,
    std::string_view request,
    std::string_view domain,
    std::string_view address,
    std::string_view identity,
    std::string_view mechanism) noexcept
    -> std::shared_future<opentxs::network::zeromq::Message>
{
    auto promise = std::promise<opentxs::network::zeromq::Message>{};
    auto socket = ot_.ZMQ().Internal().RequestSocket();

    try {
        if (false == socket->Start(main_endpoint_)) {
            throw std::runtime_error{"failed to connect to zap endpoint"};
        }

        auto result = socket->Send([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame(version.data(), version.size());
            out.AddFrame(request.data(), request.size());
            out.AddFrame(domain.data(), domain.size());
            out.AddFrame(address.data(), address.size());
            out.AddFrame(identity.data(), identity.size());
            out.AddFrame(mechanism.data(), mechanism.size());

            return out;
        }());
        promise.set_value([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame();
            auto append = [&](auto& frame) { out.AddFrame(std::move(frame)); };
            std::ranges::for_each(result.second.get(), append);

            return out;
        }());
    } catch (...) {
        promise.set_exception(std::current_exception());
    }

    return promise.get_future();
}

ZAP::~ZAP() = default;
}  // namespace ottest
