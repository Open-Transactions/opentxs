// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <string_view>

#include "ottest/Basic.hpp"

namespace ottest
{
using namespace std::literals;

class OPENTXS_EXPORT PasswordCallback final : public ot::PasswordCallback
{
public:
    static constexpr auto password_default_ = "opentxs"sv;
    static constexpr auto password_1_ = "blah foo blah foo blah"sv;
    static constexpr auto password_2_ =
        "time keeps on slippin slippin slippi"sv;

    auto runOne(
        ot::Secret& output,
        std::string_view prompt,
        std::string_view key) const noexcept -> void final;
    auto runTwo(
        ot::Secret& output,
        std::string_view prompt,
        std::string_view key) const noexcept -> void final;

    auto SetPassword(std::string_view password) noexcept -> void;

    PasswordCallback() noexcept;

private:
    ot::CString password_;
};
}  // namespace ottest
