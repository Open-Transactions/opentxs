// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <string_view>

#include "ottest/Basic.hpp"

namespace ottest
{
class OPENTXS_EXPORT CfilterListener
{
public:
    class Imp;

    using Height = ot::blockchain::block::Height;
    using Position = ot::blockchain::block::Position;
    using Future = std::future<Position>;

    auto GetFuture(const Height height) noexcept -> Future;

    CfilterListener(
        const ot::api::Session& api,
        std::string_view name) noexcept;

    ~CfilterListener();

private:
    std::shared_ptr<Imp> imp_;
};
}  // namespace ottest
