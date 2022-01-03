// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"

namespace opentxs
{
namespace client
{
class ServerAction
{
public:
    virtual auto LastSendResult() const -> SendResult = 0;
    virtual auto Reply() const -> const std::shared_ptr<Message> = 0;

    virtual auto Run(const std::size_t totalRetries = 2) -> std::string = 0;

    virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;
};
}  // namespace client
}  // namespace opentxs
