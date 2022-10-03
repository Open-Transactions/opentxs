// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::client
{
class ServerAction
{
public:
    virtual auto LastSendResult() const -> otx::client::SendResult = 0;
    virtual auto Reply() const -> const std::shared_ptr<Message> = 0;

    virtual auto Run(const std::size_t totalRetries = 2)
        -> UnallocatedCString = 0;

    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;

    virtual ~ServerAction() = default;

protected:
    ServerAction() = default;
};
}  // namespace opentxs::client
