// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTCLIENT_HPP
#define OPENTXS_CLIENT_OTCLIENT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/client/OTMessageOutbuffer.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace opentxs
{
class OTClient
{
public:
    inline OTMessageOutbuffer& GetMessageOutbuffer()
    {
        return m_MessageOutbuffer;
    }
    std::int32_t ProcessUserCommand(
        const MessageType requestedCommand,
        ServerContext& context,
        Message& theMessage,
        const identifier::Nym& pHisNymID,
        const Identifier& pHisAcctID,
        const Amount lTransactionAmount = 0,
        const Account* pAccount = nullptr,
        const UnitDefinition* pMyUnitDefinition = nullptr);
    void QueueOutgoingMessage(const Message& theMessage);

    explicit OTClient(const api::Core& api);

private:
    const api::Core& api_;
    OTMessageOutbuffer m_MessageOutbuffer;
};
}  // namespace opentxs
#endif
