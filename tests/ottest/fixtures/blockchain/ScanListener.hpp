// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "ottest/Basic.hpp"

namespace ottest
{
struct OPENTXS_EXPORT ScanListener {
    using Callback = ot::network::zeromq::ListenCallback;
    using Subaccount = ot::blockchain::crypto::Subaccount;
    using Subchain = ot::blockchain::crypto::Subchain;
    using Height = ot::blockchain::block::Height;
    using Future = std::future<void>;

    auto wait(const Future& future) const noexcept -> bool;

    auto get_future(
        const Subaccount& account,
        Subchain subchain,
        Height target) noexcept -> Future;

    ScanListener(const ot::api::Session& api) noexcept;

    ~ScanListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
