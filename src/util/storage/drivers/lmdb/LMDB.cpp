// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Driver

#include "util/storage/drivers/lmdb/LMDB.hpp"  // IWYU pragma: associated

#include <array>
#include <memory>
#include <span>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/Types.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageLMDB(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>
{
    using ReturnType = storage::driver::LMDB;

    return std::make_unique<ReturnType>(crypto, config);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
LMDB::Data::Data(const storage::Config& config) noexcept
    : table_names_({
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
}

auto LMDB::Data::get_search(Search order) noexcept -> std::array<Table, 2>
{
    using enum Bucket;
    using enum Search;

    if (ltr == order) {

        return {get_table(left), get_table(right)};
    } else {

        return {get_table(right), get_table(left)};
    }
}

auto LMDB::Data::get_table(Bucket bucket) noexcept -> Table
{
    using enum Bucket;

    switch (bucket) {
        case left: {

            return Table::A;
        }
        case right: {

            return Table::B;
        }
        default: {
            LogAbort()(OT_PRETTY_STATIC(Data))("invalid bucket").Abort();
        }
    }
}
}  // namespace opentxs::storage::driver

namespace opentxs::storage::driver
{
using namespace std::literals;

LMDB::LMDB(const api::Crypto& crypto, const storage::Config& config)
    : Driver(crypto, config)
    , data_(config_)
{
}

auto LMDB::Commit(const Hash& root, Transaction values, Bucket bucket)
    const noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto& lmdb = data.lmdb_;
    auto tx = lmdb.TransactionRW();
    using enum Data::Table;
    const auto rc =
        lmdb.Store(Control, config_.lmdb_root_key_, unencoded_view(root), tx)
            .first;

    if (false == rc) { return false; }

    if (false == store(data, tx, values, bucket)) { return false; }

    return tx.Finalize(true);
}

auto LMDB::Description() const noexcept -> std::string_view { return "lmdb"sv; }

auto LMDB::EmptyBucket(Bucket bucket) const noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return data.lmdb_.Delete(data.get_table(bucket));
}

auto LMDB::Load(const Log& logger, const Hash& key, Search order, Writer& out)
    const noexcept -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    const auto search = data.get_search(order);
    auto cb = [&out](const auto& i) mutable { copy(i, std::move(out)); };

    for (const auto& table : search) {
        if (data.lmdb_.Load(table, unencoded_view(key), cb)) { return true; }
    }

    return false;
}

auto LMDB::LoadRoot() const noexcept -> Hash
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    auto out = Hash{};
    using enum Data::Table;
    data.lmdb_.Load(
        Control, config_.lmdb_root_key_, [&](const auto val) -> void {
            out = read(val);
        });

    return out;
}

auto LMDB::Store(Transaction values, Bucket bucket) const noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto& lmdb = data.lmdb_;
    auto tx = lmdb.TransactionRW();
    using enum Data::Table;

    if (false == store(data, tx, values, bucket)) { return false; }

    return tx.Finalize(true);
}

auto LMDB::store(
    Data& data,
    lmdb::Transaction& tx,
    Transaction values,
    Bucket bucket) const noexcept -> bool
{
    if (values.first.empty()) { return true; }

    auto& lmdb = data.lmdb_;
    using enum Data::Table;
    const auto count = values.first.size();
    const auto table = data.get_table(bucket);

    for (auto n = 0_uz; n < count; ++n) {
        const auto& key = values.first[n];
        const auto& value = values.second[n];
        const auto rc = lmdb.Store(table, unencoded_view(key), value, tx).first;

        if (false == rc) { return false; }
    }

    return true;
}

LMDB::~LMDB() = default;
}  // namespace opentxs::storage::driver
