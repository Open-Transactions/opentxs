// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Config.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}
#include <string_view>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::database::common
{
Configuration::Configuration(
    const api::Session& api,
    storage::lmdb::Database& lmdb) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , config_table_(Table::ConfigMulti)
    , socket_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto rc =
            out->Start(api_.Endpoints().BlockchainSyncServerUpdated().data());

        assert_true(rc);

        return out;
    }())
{
}

auto Configuration::AddSyncServer(std::string_view endpoint) const noexcept
    -> bool
{
    if (endpoint.empty()) { return false; }

    const auto [success, code] = lmdb_.Store(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        endpoint,
        MDB_NODUPDATA);

    if (success) {
        LogDetail()()("successfully added endpoint ")(endpoint).Flush();
        static const auto value = bool{true};
        socket_->Send([&] {
            auto work = network::zeromq::tagged_message(
                WorkType::SyncServerUpdated, true);
            work.AddFrame(endpoint.data(), endpoint.size());
            work.AddFrame(value);

            return work;
        }());

        return true;
    }

    return MDB_KEYEXIST == code;
}

auto Configuration::DeleteSyncServer(std::string_view endpoint) const noexcept
    -> bool
{
    if (endpoint.empty()) { return false; }

    const auto output = lmdb_.Delete(
        config_table_, tsv(Database::Key::SyncServerEndpoint), endpoint);

    if (output) {
        static constexpr auto deleted{false};
        socket_->Send([&] {
            auto work = network::zeromq::tagged_message(
                WorkType::SyncServerUpdated, true);
            work.AddFrame(endpoint.data(), endpoint.size());
            work.AddFrame(deleted);

            return work;
        }());

        return true;
    }

    return true;
}

auto Configuration::GetSyncServers(alloc::Default alloc) const noexcept
    -> Endpoints
{
    auto output = Endpoints{alloc};
    lmdb_.Load(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        [&](const auto view) { output.emplace_back(view); },
        storage::lmdb::Mode::Multiple);

    return output;
}
}  // namespace opentxs::blockchain::database::common
