// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <string_view>

#include "ottest/fixtures/common/Base.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class CustomZAPHandler;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
using namespace std::literals;

class OPENTXS_EXPORT ZAP : public Base
{
public:
    static constexpr auto main_endpoint_ = "inproc://zeromq.zap.01"sv;
    static constexpr auto alt_endpoint_1_ = "inproc://ottest.zap.01"sv;
    static constexpr auto alt_endpoint_2_ = "inproc://ottest.zap.02"sv;
    static constexpr auto version_ = "1.0"sv;
    static constexpr auto request_ = "1"sv;
    static constexpr auto domain_ = "domain"sv;
    static constexpr auto alt_domain_ = "other domain"sv;
    static constexpr auto address_ = "127.0.0.1"sv;
    static constexpr auto identity_ = "BOB"sv;
    static constexpr auto mechanism_ = "NULL"sv;
    static constexpr auto success_status_ = "200"sv;
    static constexpr auto fail_status_ = "400"sv;

    auto MakeHandler(
        std::string_view domain,
        std::string_view endpoint,
        bool accept) noexcept -> std::shared_ptr<CustomZAPHandler>;
    auto SendRequest(
        std::string_view version,
        std::string_view request,
        std::string_view domain,
        std::string_view address,
        std::string_view identity,
        std::string_view mechanism) noexcept
        -> std::shared_future<opentxs::network::zeromq::Message>;

    ZAP();

    ~ZAP() override;
};
}  // namespace ottest
