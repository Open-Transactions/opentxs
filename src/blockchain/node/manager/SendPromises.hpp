// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <utility>

#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::manager
{
class SendPromises
{
public:
    auto finish(int index) noexcept -> std::promise<SendOutcome>;
    auto get() noexcept -> std::pair<int, Manager::PendingOutgoing>;

    SendPromises() noexcept;

private:
    int counter_;
    Map<int, std::promise<SendOutcome>> map_;
};
}  // namespace opentxs::blockchain::node::manager
