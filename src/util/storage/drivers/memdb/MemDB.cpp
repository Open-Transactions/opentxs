// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "util/storage/drivers/memdb/MemDB.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto StorageMemDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& parent,
    const storage::Config& config,
    const Flag& bucket) noexcept -> std::unique_ptr<storage::Plugin>
{
    using ReturnType = storage::driver::MemDB;

    return std::make_unique<ReturnType>(crypto, asio, parent, config, bucket);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
MemDB::MemDB(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Storage& storage,
    const storage::Config& config,
    const Flag& bucket)
    : ot_super(crypto, asio, storage, config, bucket)
    , root_("")
    , a_()
    , b_()
{
}

auto MemDB::EmptyBucket(const bool bucket) const -> bool
{
    eLock lock(shared_lock_);

    if (bucket) {
        a_.clear();
    } else {
        b_.clear();
    }

    return true;
}

auto MemDB::LoadFromBucket(
    const UnallocatedCString& key,
    UnallocatedCString& value,
    const bool bucket) const -> bool
{
    sLock lock(shared_lock_);

    try {
        if (bucket) {
            value = a_.at(key);
        } else {
            value = b_.at(key);
        }
    } catch (...) {

        return false;
    }

    return (false == value.empty());
}

auto MemDB::LoadRoot() const -> UnallocatedCString
{
    sLock lock(shared_lock_);

    return root_;
}

void MemDB::store(
    [[maybe_unused]] const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (bucket) {
        a_[key] = value;
    } else {
        b_[key] = value;
    }

    promise->set_value(true);
}

auto MemDB::StoreRoot(
    [[maybe_unused]] const bool commit,
    const UnallocatedCString& hash) const -> bool
{
    eLock lock(shared_lock_);
    root_ = hash;

    return true;
}
}  // namespace opentxs::storage::driver
