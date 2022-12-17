// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "util/storage/lmdb/DatabasePrivate.hpp"

extern "C" {
using MDB_env = struct MDB_env;
using MDB_txn = struct MDB_txn;
}

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace lmdb
{
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::lmdb
{
class TransactionPrivate
{
public:
    operator MDB_txn*() noexcept { return ptr_; }

    bool success_;
    WriteHandle write_;
    MDB_txn* ptr_;

    auto Finalize(const std::optional<bool> success) noexcept -> bool;

    TransactionPrivate() noexcept;
    TransactionPrivate(MDB_env* env) noexcept(false);
    TransactionPrivate(MDB_env* env, Transaction& parent) noexcept(false);
    TransactionPrivate(const TransactionPrivate&) = delete;
    TransactionPrivate(TransactionPrivate&&) = delete;
    auto operator=(const TransactionPrivate&) -> TransactionPrivate& = delete;
    auto operator=(TransactionPrivate&&) -> TransactionPrivate& = delete;

    ~TransactionPrivate();

private:
    TransactionPrivate(
        MDB_env* env,
        WriteHandle write,
        MDB_txn* parent) noexcept(false);
};
}  // namespace opentxs::storage::lmdb
