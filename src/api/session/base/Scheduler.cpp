// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "api/session/base/Scheduler.hpp"  // IWYU pragma: associated

#include <ctime>
#include <limits>
#include <ratio>

#include "internal/api/network/Dht.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace opentxs::api::session
{
Scheduler::Scheduler(const api::Context& parent, Flag& running)
    : Lockable()
    , parent_(parent)
    , nym_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , nym_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , server_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , server_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_publish_interval_(std::numeric_limits<std::int64_t>::max())
    , unit_refresh_interval_(std::numeric_limits<std::int64_t>::max())
    , running_(running)
    , periodic_()
{
}

void Scheduler::Start(
    const api::session::Storage* const storage,
    const api::network::Dht& dht)
{
    OT_ASSERT(nullptr != storage);

    const auto now = std::chrono::seconds(std::time(nullptr));

    Schedule(
        std::chrono::seconds(nym_publish_interval_),
        [=, &dht]() -> void {
            NymLambda nymLambda(
                [=, &dht](const identity::Nym::Serialized& nym) -> void {
                    dht.Internal().Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(nym_refresh_interval_),
        [=, &dht]() -> void {
            NymLambda nymLambda(
                [=, &dht](const identity::Nym::Serialized& nym) -> void {
                    dht.GetPublicNym(nym.nymid());
                });
            storage->MapPublicNyms(nymLambda);
        },
        (now - std::chrono::seconds(nym_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(server_publish_interval_),
        [=, &dht]() -> void {
            ServerLambda serverLambda(
                [=, &dht](const proto::ServerContract& server) -> void {
                    dht.Internal().Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(server_refresh_interval_),
        [=, &dht]() -> void {
            ServerLambda serverLambda(
                [=, &dht](const proto::ServerContract& server) -> void {
                    dht.GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - std::chrono::seconds(server_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(unit_publish_interval_),
        [=, &dht]() -> void {
            UnitLambda unitLambda(
                [=, &dht](const proto::UnitDefinition& unit) -> void {
                    dht.Internal().Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(unit_refresh_interval_),
        [=, &dht]() -> void {
            UnitLambda unitLambda(
                [=, &dht](const proto::UnitDefinition& unit) -> void {
                    dht.GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - std::chrono::seconds(unit_refresh_interval_) / 2));

    periodic_ = std::thread(&Scheduler::thread, this);
}

void Scheduler::thread()
{
    while (running_) {
        // Storage has its own interval checking.
        storage_gc_hook();
        Sleep(100ms);
    }
}

Scheduler::~Scheduler()
{
    if (periodic_.joinable()) { periodic_.join(); }
}
}  // namespace opentxs::api::session
