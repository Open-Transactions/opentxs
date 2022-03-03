// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Config.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}
#include <string_view>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database::common
{
Configuration::Configuration(
    const api::Session& api,
    storage::lmdb::LMDB& lmdb) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , config_table_(Table::ConfigMulti)
    , socket_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto rc =
            out->Start(api_.Endpoints().BlockchainSyncServerUpdated().data());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto Configuration::AddSyncServer(
    const UnallocatedCString& endpoint) const noexcept -> bool
{
    if (endpoint.empty()) { return false; }

    const auto [success, code] = lmdb_.Store(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        endpoint,
        nullptr,
        MDB_NODUPDATA);

    if (success) {
        LogDetail()(OT_PRETTY_CLASS())("successfully added endpoint ")(endpoint)
            .Flush();
        static const auto value = bool{true};
        socket_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::SyncServerUpdated);
            work.AddFrame(endpoint);
            work.AddFrame(value);

            return work;
        }());

        return true;
    }

    return MDB_KEYEXIST == code;
}

auto Configuration::DeleteSyncServer(
    const UnallocatedCString& endpoint) const noexcept -> bool
{
    if (endpoint.empty()) { return false; }

    const auto output = lmdb_.Delete(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        endpoint,
        nullptr);

    if (output) {
        static constexpr auto deleted{false};
        socket_->Send([&] {
            auto work =
                network::zeromq::tagged_message(WorkType::SyncServerUpdated);
            work.AddFrame(endpoint);
            work.AddFrame(deleted);

            return work;
        }());

        return true;
    }

    return true;
}

auto Configuration::GetSyncServers() const noexcept -> Endpoints
{
    auto output = Endpoints{};
    lmdb_.Load(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        [&](const auto view) { output.emplace_back(view); },
        storage::lmdb::LMDB::Mode::Multiple);

    return output;
}
}  // namespace opentxs::blockchain::database::common
