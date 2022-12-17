// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
// LMDB implementation of opentxs::storage
class LMDB final : public virtual implementation::Plugin,
                   public virtual storage::Driver
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> UnallocatedCString final;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool final;

    void Cleanup() final;
    void Cleanup_LMDB();

    LMDB(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);
    LMDB() = delete;
    LMDB(const LMDB&) = delete;
    LMDB(LMDB&&) = delete;
    auto operator=(const LMDB&) -> LMDB& = delete;
    auto operator=(LMDB&&) -> LMDB& = delete;

    ~LMDB() final;

private:
    using ot_super = Plugin;

    enum Table {
        Control = 0,
        A = 1,
        B = 2,
    };

    const lmdb::TableNames table_names_;
    lmdb::Database lmdb_;

    auto get_table(const bool bucket) const -> Table;
    void store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket,
        std::promise<bool>* promise) const final;

    void Init_LMDB();
};
}  // namespace opentxs::storage::driver
