// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/Database.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <memory>

#include "internal/blockchain/database/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainDatabase(
    const api::Session& api,
    const blockchain::node::Endpoints& endpoints,
    const blockchain::database::common::Database& common,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter) noexcept
    -> std::shared_ptr<blockchain::database::Database>
{
    using ReturnType = blockchain::database::implementation::Database;

    return std::make_shared<ReturnType>(api, endpoints, common, chain, filter);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::database::implementation
{
const VersionNumber Database::db_version_{2};
const storage::lmdb::TableNames Database::table_names_{
    {database::Config, "config"},
    {database::BlockHeaderMetadata, "block_header_metadata"},
    {database::BlockHeaderBest, "best_header_chain"},
    {database::ChainData, "block_header_data"},
    {database::BlockHeaderSiblings, "block_siblings"},
    {database::BlockHeaderDisconnected, "disconnected_block_headers"},
    {database::BlockFilterBest, "filter_tips"},
    {database::BlockFilterHeaderBest, "filter_header_tips"},
    {database::Proposals, "proposals"},
    {database::SubchainLastIndexed, "subchain_last_indexed"},
    {database::SubchainLastScanned, "subchain_last_scanned"},
    {database::SubchainIDTable, "subchain_id"},
    {database::WalletPatterns, "wallet_patterns"},
    {database::SubchainPatterns, "subchain_patterns"},
    {database::SubchainMatches, "subchain_matches"},
    {database::WalletOutputs, "wallet_outputs"},
    {database::AccountOutputs, "account_outputs"},
    {database::NymOutputs, "nym_outputs"},
    {database::PositionOutputs, "position_outputs"},
    {database::ProposalCreatedOutputs, "proposal_created_outputs"},
    {database::ProposalSpentOutputs, "proposal_spent_outputs"},
    {database::OutputProposals, "output_proposals"},
    {database::StateOutputs, "state_outputs"},
    {database::SubchainOutputs, "subchain_outputs"},
    {database::KeyOutputs, "key_outputs"},
    {database::GenerationOutputs, "generation_outputs"},
};

Database::Database(
    const api::Session& api,
    const node::Endpoints& endpoints,
    const database::common::Database& common,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter) noexcept
    : api_(api)
    , chain_(chain)
    , common_(common)
    , lmdb_([&] {
        using enum Table;
        auto lmdb = storage::lmdb::Database{
            table_names_,
            common.AllocateStorageFolder(
                std::to_string(static_cast<std::uint32_t>(chain_))),
            {
                {Config, MDB_INTEGERKEY},
                {BlockHeaderMetadata, 0},
                {BlockHeaderBest, MDB_INTEGERKEY},
                {ChainData, MDB_INTEGERKEY},
                {BlockHeaderSiblings, 0},
                {BlockHeaderDisconnected, MDB_DUPSORT},
                {BlockFilterBest, MDB_INTEGERKEY},
                {BlockFilterHeaderBest, MDB_INTEGERKEY},
                {Proposals, 0},
                {SubchainLastIndexed, 0},
                {SubchainLastScanned, 0},
                {SubchainIDTable, 0},
                {WalletPatterns, MDB_DUPSORT},
                {SubchainPatterns, MDB_DUPSORT},
                {SubchainMatches, MDB_DUPSORT},
                {WalletOutputs, 0},
                {AccountOutputs, MDB_DUPSORT},
                {NymOutputs, MDB_DUPSORT},
                {PositionOutputs, MDB_DUPSORT | MDB_DUPFIXED},
                {ProposalCreatedOutputs, MDB_DUPSORT},
                {ProposalSpentOutputs, MDB_DUPSORT},
                {OutputProposals, 0},
                {StateOutputs, MDB_DUPSORT | MDB_DUPFIXED},
                {SubchainOutputs, MDB_DUPSORT},
                {KeyOutputs, MDB_DUPSORT},
                {GenerationOutputs, MDB_DUPSORT | MDB_DUPFIXED},
            },
            0};

        return lmdb;
    }())
    , original_version_(get_original_version(lmdb_))
    , current_version_(get_current_version(original_version_, lmdb_))
    , blocks_(api_, lmdb_, chain_)
    , filters_(api_, common_, lmdb_, chain_)
    , headers_(api_, endpoints, common_, lmdb_, chain_)
    , wallet_(api_, common_, lmdb_, chain_, filter)
    , sync_(api_, common_, lmdb_, chain_)
{
}

auto Database::get_current_version(
    const VersionNumber& original,
    storage::lmdb::Database& db) noexcept -> VersionNumber
{
    auto version = db_version_;
    init_db(version, original, db);

    return version;
}

auto Database::get_original_version(storage::lmdb::Database& db) noexcept
    -> VersionNumber
{
    if (db.Exists(database::Config, tsv(database::Key::Version))) {
        auto version = VersionNumber{};
        db.Load(
            database::Config,
            tsv(database::Key::Version),
            [&](const auto bytes) {
                if (valid(bytes)) {
                    std::memcpy(
                        std::addressof(version),
                        bytes.data(),
                        std::min(bytes.size(), sizeof(version)));
                }
            });

        return version;
    } else {
        const auto stored = db.Store(
            database::Config, tsv(database::Key::Version), tsv(db_version_));

        OT_ASSERT(stored.first);

        return db_version_;
    }
}

auto Database::init_db(
    const VersionNumber target,
    const VersionNumber current,
    storage::lmdb::Database& db) noexcept -> void
{
    if (current < target) {
        switch (current) {
            case 1u: {
                using enum Table;
                using enum Key;
                static const auto tables = storage::lmdb::TablesToInit{
                    {AccountOutputs, MDB_DUPSORT},
                    {GenerationOutputs, MDB_DUPSORT | MDB_DUPFIXED},
                    {KeyOutputs, MDB_DUPSORT},
                    {NymOutputs, MDB_DUPSORT},
                    {OutputProposals, 0},
                    {PositionOutputs, MDB_DUPSORT | MDB_DUPFIXED},
                    {ProposalCreatedOutputs, MDB_DUPSORT},
                    {ProposalSpentOutputs, MDB_DUPSORT},
                    {Proposals, 0},
                    {StateOutputs, MDB_DUPSORT | MDB_DUPFIXED},
                    {SubchainIDTable, 0},
                    {SubchainLastIndexed, 0},
                    {SubchainLastScanned, 0},
                    {SubchainMatches, MDB_DUPSORT},
                    {SubchainOutputs, MDB_DUPSORT},
                    {SubchainPatterns, MDB_DUPSORT},
                    {WalletOutputs, 0},
                    {WalletPatterns, MDB_DUPSORT},
                };
                static constexpr auto keys = {
                    WalletPosition,
                };
                auto tx = db.TransactionRW();
                db.PurgeTables(tables, tx);

                for (const auto& key : keys) {
                    db.Delete(Config, tsv(key), tx);
                }

                tx.Finalize(true);
                [[fallthrough]];
            }
            default: {
            }
        }
    }
}

auto Database::StartReorg() noexcept -> storage::lmdb::Transaction
{
    return lmdb_.TransactionRW();
}
}  // namespace opentxs::blockchain::database::implementation
