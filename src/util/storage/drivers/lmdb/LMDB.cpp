// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "util/storage/drivers/lmdb/LMDB.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageLMDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>
{
    using ReturnType = storage::driver::LMDB;

    return std::make_unique<ReturnType>(crypto, asio, parent, config, bucket);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
LMDB::LMDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const Flag& bucket)
    : ot_super(crypto, asio, storage, config, bucket)
    , table_names_({
          {Table::Control, config.lmdb_control_table_},
          {Table::A, config.lmdb_primary_bucket_},
          {Table::B, config.lmdb_secondary_bucket_},
      })
    , lmdb_(
          table_names_,
          config.path_,
          {
              {Table::Control, 0},
              {Table::A, 0},
              {Table::B, 0},
          })
{
    LogVerbose()(OT_PRETTY_CLASS())("Using ")(config_.path_).Flush();
    Init_LMDB();
}

void LMDB::Cleanup() { Cleanup_LMDB(); }

void LMDB::Cleanup_LMDB() {}

auto LMDB::EmptyBucket(const bool bucket) const -> bool
{
    return lmdb_.Delete(get_table(bucket));
}

auto LMDB::get_table(const bool bucket) const -> LMDB::Table
{
    return (bucket) ? Table::A : Table::B;
}

void LMDB::Init_LMDB()
{
    LogVerbose()(OT_PRETTY_CLASS())("Database initialized.").Flush();
}

auto LMDB::LoadFromBucket(
    const UnallocatedCString& key,
    UnallocatedCString& value,
    const bool bucket) const -> bool
{
    value = {};
    lmdb_.Load(
        get_table(bucket), key, [&](const auto data) -> void { value = data; });

    return false == value.empty();
}

auto LMDB::LoadRoot() const -> UnallocatedCString
{
    auto output = UnallocatedCString{};
    lmdb_.Load(
        Table::Control, config_.lmdb_root_key_, [&](const auto data) -> void {
            output = data;
        });

    return output;
}

void LMDB::store(
    const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    if (isTransaction) {
        lmdb_.Queue(get_table(bucket), key, value);
        promise->set_value(true);
    } else {
        const auto output = lmdb_.Store(get_table(bucket), key, value);
        promise->set_value(output.first);
    }
}

auto LMDB::StoreRoot(const bool commit, const UnallocatedCString& hash) const
    -> bool
{
    if (commit) {
        if (lmdb_.Queue(Table::Control, config_.lmdb_root_key_, hash)) {

            return lmdb_.Commit();
        }

        return false;
    } else {

        return lmdb_.Store(Table::Control, config_.lmdb_root_key_, hash).first;
    }
}

LMDB::~LMDB() { Cleanup_LMDB(); }
}  // namespace opentxs::storage::driver
