// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "util/storage/lmdb/TransactionPrivate.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"

namespace opentxs::storage::lmdb
{
TransactionPrivate::TransactionPrivate() noexcept
    : success_(false)
    , write_(nullptr)
    , ptr_(nullptr)
{
}

TransactionPrivate::TransactionPrivate(MDB_env* env) noexcept(false)
    : TransactionPrivate(env, nullptr, nullptr)
{
}

TransactionPrivate::TransactionPrivate(
    MDB_env* env,
    Transaction& parent) noexcept(false)
    : TransactionPrivate(env, parent.imp_->write_, parent.imp_->ptr_)
{
}

TransactionPrivate::TransactionPrivate(
    MDB_env* env,
    std::shared_ptr<GuardedWrite::handle> write,
    MDB_txn* parent) noexcept(false)
    : success_(false)
    , write_(std::move(write))
    , ptr_(nullptr)
{
    const auto flags = Flags{write_ ? 0u : MDB_RDONLY};

    if (0 != ::mdb_txn_begin(env, parent, flags, &ptr_)) {
        throw std::runtime_error("Failed to start transaction");
    }
}

auto TransactionPrivate::Finalize(const std::optional<bool> success) noexcept
    -> bool
{
    struct Cleanup {
        Cleanup(MDB_txn*& ptr)
            : ptr_(ptr)
        {
        }

        ~Cleanup() { ptr_ = nullptr; }

    private:
        MDB_txn*& ptr_;
    };

    if (nullptr != ptr_) {
        if (success.has_value()) { success_ = success.value(); }

        auto cleanup = Cleanup{ptr_};

        if (success_) {

            return 0 == ::mdb_txn_commit(ptr_);
        } else {
            ::mdb_txn_abort(ptr_);

            return true;
        }
    }

    return false;
}

TransactionPrivate::~TransactionPrivate() { Finalize(std::nullopt); }
}  // namespace opentxs::storage::lmdb
