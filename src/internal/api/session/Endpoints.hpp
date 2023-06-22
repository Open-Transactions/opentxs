// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs::api::session::internal
{
class Endpoints : virtual public api::session::Endpoints
{
public:
    static auto Asio() noexcept -> std::string_view;
    static auto ContextShutdown() noexcept -> std::string_view;

    virtual auto BlockchainMessageRouter() const noexcept
        -> std::string_view = 0;
    virtual auto BlockchainReportStatus() const noexcept
        -> std::string_view = 0;
    virtual auto BlockchainStartupPublish() const noexcept
        -> std::string_view = 0;
    virtual auto BlockchainStartupPull() const noexcept -> std::string_view = 0;
    virtual auto BlockchainSyncChecksumFailure() const noexcept
        -> std::string_view = 0;
    auto Internal() const noexcept -> const Endpoints& final { return *this; }
    virtual auto OTDHTBlockchain(opentxs::blockchain::Type chain) const noexcept
        -> std::string_view = 0;
    virtual auto OTDHTNodePublish() const noexcept -> std::string_view = 0;
    virtual auto OTDHTNodePull() const noexcept -> std::string_view = 0;
    virtual auto OTDHTNodeRouter() const noexcept -> std::string_view = 0;
    virtual auto OTDHTWallet() const noexcept -> std::string_view = 0;
    virtual auto PairEvent() const noexcept -> std::string_view = 0;
    virtual auto PeerReplyUpdate() const noexcept -> std::string_view = 0;
    virtual auto PeerRequestUpdate() const noexcept -> std::string_view = 0;
    virtual auto PendingBailment() const noexcept -> std::string_view = 0;
    virtual auto ProcessPushNotification() const noexcept
        -> std::string_view = 0;
    virtual auto PushNotification() const noexcept -> std::string_view = 0;

    auto Internal() noexcept -> Endpoints& final { return *this; }

    ~Endpoints() override = default;
};
}  // namespace opentxs::api::session::internal
