// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <memory>
#include <string_view>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
struct Listener;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

class OPENTXS_EXPORT SyncServerDB : public ::testing::Test
{
protected:
    using Endpoints = ot::Vector<ot::CString>;

    static constexpr auto first_server_{"tcp://example.com:1"};
    static constexpr auto second_server_{"tcp://example.com:2"};
    static constexpr auto other_server_{"tcp://example.com:3"};
    static std::unique_ptr<Listener> listener_p_;

    const ot::api::session::Client& api_;
    Listener& listener_;

    static auto count(
        const Endpoints& endpoints,
        std::string_view value) noexcept -> std::size_t;
    void cleanup() noexcept;

    SyncServerDB();
};
}  // namespace ottest
