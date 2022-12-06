// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/lmdb/Transaction.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "util/storage/lmdb/TransactionPrivate.hpp"

namespace opentxs::storage::lmdb
{
Transaction::Transaction(std::unique_ptr<TransactionPrivate> imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(imp_);
}

Transaction::Transaction() noexcept
    : Transaction(std::make_unique<TransactionPrivate>())
{
}

Transaction::Transaction(Transaction&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

auto Transaction::Finalize(const std::optional<bool> success) noexcept -> bool
{
    return imp_->Finalize(std::move(success));
}

auto Transaction::Success() const noexcept -> bool { return imp_->success_; }

auto Transaction::SetSuccess(bool value) noexcept -> void
{
    imp_->success_ = value;
}

Transaction::~Transaction()
{
    if (imp_) { Finalize(); }
}
}  // namespace opentxs::storage::lmdb
