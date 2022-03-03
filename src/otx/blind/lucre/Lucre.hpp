// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: begin_exports
#include <lucre/bank.h>
// IWYU pragma: end_exports
#include <memory>

#include "opentxs/util/Container.hpp"

namespace opentxs::otx::blind
{
class LucreDumper
{
public:
    static auto IsEnabled() noexcept -> bool;

    LucreDumper();

    ~LucreDumper();

private:
    class Imp;

    auto init() noexcept -> void;
    auto log_to_file() noexcept -> void;
    auto log_to_screen() noexcept -> void;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::otx::blind
