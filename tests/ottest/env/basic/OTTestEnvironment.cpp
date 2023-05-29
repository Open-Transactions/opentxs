// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/env/OTTestEnvironment.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <functional>
#include <optional>
#include <stdexcept>

#include "ottest/Basic.hpp"

namespace ottest
{
static auto get_ot() noexcept
    -> std::optional<std::reference_wrapper<const opentxs::api::Context>>&
{
    static auto data =
        std::optional<std::reference_wrapper<const opentxs::api::Context>>{
            std::nullopt};

    return data;
}
}  // namespace ottest

namespace ottest
{
OTTestEnvironment::OTTestEnvironment() noexcept
{
    auto& args = const_cast<ot::Options&>(ottest::Args(false));
    args.SetQtRootObject(GetQT());
}

auto OTTestEnvironment::GetOT() -> const opentxs::api::Context&
{
    if (const auto& ot = get_ot(); ot.has_value()) {

        return ot->get();
    } else {

        throw std::runtime_error{"Environment not initialized"};
    }
}

auto OTTestEnvironment::SetUp() -> void
{
    get_ot().emplace(ot::InitContext(Args(false)));
}

auto OTTestEnvironment::TearDown() -> void
{
    ot::Cleanup();
    get_ot().reset();
    WipeHome();
    StopQT();
}

OTTestEnvironment::~OTTestEnvironment() = default;
}  // namespace ottest
