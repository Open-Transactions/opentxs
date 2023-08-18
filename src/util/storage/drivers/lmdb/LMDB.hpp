// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <array>
#include <shared_mutex>
#include <string_view>

#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/drivers/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
namespace lmdb
{
class Transaction;
}  // namespace lmdb

class Config;
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class LMDB final : public virtual storage::implementation::Driver
{
public:
    auto Description() const noexcept -> std::string_view final;
    auto Load(const Log& logger, const Hash& key, Search order, Writer& value)
        const noexcept -> bool final;
    auto LoadRoot() const noexcept -> Hash final;

    auto Commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> bool final;
    auto EmptyBucket(Bucket bucket) const noexcept -> bool final;
    auto Store(Transaction data, Bucket bucket) const noexcept -> bool final;

    LMDB(const api::Crypto& crypto, const storage::Config& config);
    LMDB() = delete;
    LMDB(const LMDB&) = delete;
    LMDB(LMDB&&) = delete;
    auto operator=(const LMDB&) -> LMDB& = delete;
    auto operator=(LMDB&&) -> LMDB& = delete;

    ~LMDB() final;

private:
    struct Data {
    private:
        const lmdb::TableNames table_names_;

    public:
        enum Table {
            Control = 0,
            A = 1,
            B = 2,
        };

        lmdb::Database lmdb_;

        static auto get_search(Search order) noexcept -> std::array<Table, 2>;
        static auto get_table(Bucket bucket) noexcept -> Table;

        Data(const storage::Config& config) noexcept;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    mutable GuardedData data_;

    auto store(
        Data& data,
        lmdb::Transaction& tx,
        Transaction values,
        Bucket bucket) const noexcept -> bool;
};
}  // namespace opentxs::storage::driver
