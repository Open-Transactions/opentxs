// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <functional>
#include <future>
#include <memory>
#include <utility>

namespace ottest
{
class OPENTXS_EXPORT PeerRequestListener
{
public:
    using Future = std::future<opentxs::contract::peer::Reply>;
    using Callback = std::function<void(opentxs::contract::peer::Request&&)>;

    auto get() noexcept -> Future { return std::move(future_); }

    PeerRequestListener(
        const opentxs::api::Session& requestor,
        const opentxs::api::Session& responder,
        Callback cb) noexcept;

    ~PeerRequestListener();

private:
    using Promise = std::promise<opentxs::contract::peer::Reply>;

    Future future_;

    PeerRequestListener(
        const opentxs::api::Session& requestor,
        const opentxs::api::Session& responder,
        Callback cb,
        std::shared_ptr<Promise> promise) noexcept;
};
}  // namespace ottest
