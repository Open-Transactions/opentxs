// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <QString>

#pragma once

#include <functional>
#include <future>
#include <memory>

#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/Types.hpp"

class QString;

namespace opentxs::ui::implementation
{
class SendMonitor
{
public:
    using Future = std::future<blockchain::node::SendOutcome>;
    using Callback = std::function<void(int, int, QString)>;

    auto shutdown() noexcept -> void;
    auto watch(Future&& future, Callback&& cb) noexcept -> int;

    SendMonitor() noexcept;
    SendMonitor(const SendMonitor&) = delete;
    SendMonitor(SendMonitor&&) = delete;
    auto operator=(const SendMonitor&) -> SendMonitor& = delete;
    auto operator=(SendMonitor&&) -> SendMonitor& = delete;

    ~SendMonitor();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::ui::implementation
