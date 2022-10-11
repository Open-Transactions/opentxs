// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "internal/otx/Types.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace contract
{
class Unit;
}  // namespace contract

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Account;
class Message;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTClient
{
public:
    auto ProcessUserCommand(
        const MessageType requestedCommand,
        otx::context::Server& context,
        Message& theMessage,
        const identifier::Generic& pHisNymID,
        const identifier::Generic& pHisAcctID,
        const PasswordPrompt& reason,
        const Amount& lTransactionAmount = 0,
        const Account* pAccount = nullptr,
        const contract::Unit* pMyUnitDefinition = nullptr) -> std::int32_t;

    explicit OTClient(const api::Session& api);

protected:
    const api::Session& api_;
};
}  // namespace opentxs
